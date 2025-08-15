#include "net_impl_win.h"

NET_WIN32_CommandBuffer* _n_win32_command_buffer;
NET_WIN32_Context*       _n_win32_ctx;

internal void
net_init(NET_OnMessageEventHandlerType* on_message)
{
    if (_n_win32_ctx)
    {
        log_error("[NET] Network layer is already initialized");
    }

    WSADATA wsa_data;

    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
    {
        _net_win32_log_wsa_error_message();
        log_error("[NET] failed WSAStartup during net_init");
        return;
    }

    if (LOBYTE(wsa_data.wVersion) != 2 || HIBYTE(wsa_data.wVersion) != 2)
    {
        _net_win32_log_wsa_error_message();
        log_error("[NET] version 2.2 of winsock is not available.");
        WSACleanup();
        return;
    }

    _n_w32_perm_arena  = arena_new_reserve(mb(8));
    _n_w32_frame_arena = arena_new_reserve(mb(8));
    _n_win32_ctx       = arena_push_struct_zero(_n_w32_perm_arena, NET_WIN32_Context);

    _n_win32_command_buffer           = arena_push_struct_zero(_n_w32_perm_arena, NET_WIN32_CommandBuffer);
    _n_win32_command_buffer->rw_mutex = os_rw_mutex_alloc();
    _n_win32_command_buffer->cv       = os_condition_variable_alloc();

    if (on_message)
    {
        _n_win32_ctx->on_message = on_message;
    }

    os_thread_launch(_net_win32_network_main_thread, 0);
}

internal void
net_cleanup()
{
    if (WSACleanup() != 0)
    {
        _net_win32_log_wsa_error_message();
        log_error("[NET] failed WSACleanup");
        return;
    }
}

internal String
net_get_ip(Arena* arena)
{
    String             ip = string_new(arena, INET_ADDRSTRLEN);
    struct sockaddr_in socket_addr;
    inet_ntop(AF_INET, &(socket_addr.sin_addr), ip.value, INET_ADDRSTRLEN);
    return ip;
}

internal void
net_host_to_ip(Arena* arena, String address, String port)
{
    struct addrinfo hints = {0};
    hints.ai_family       = AF_UNSPEC;   // IPv4 or IPv6
    hints.ai_socktype     = SOCK_STREAM; // TCP
    hints.ai_flags        = AI_PASSIVE;

    struct addrinfo* dst_info;
    int32            status = getaddrinfo(address.value, port.value, &hints, &dst_info);
    if (status != 0)
    {
        _net_win32_log_wsa_error_message();
        log_error("[NET] Couldn't get address info for, %s:%s, status: %d", address.value, port.value, status);
        return;
    }

    String ip = string_new(arena, INET6_ADDRSTRLEN);

    struct addrinfo* node;
    for (node = dst_info; node != NULL; node = dst_info->ai_next)
    {
        void*  addr;
        String ipver;

        if (node->ai_family == AF_INET)
        {
            struct sockaddr_in* ipv4 = (struct sockaddr_in*)node->ai_addr;
            addr                     = &(ipv4->sin_addr);
            ipver                    = string("IPv4");
        }
        else
        {
            struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)node->ai_addr;
            addr                      = &(ipv6->sin6_addr);
            ipver                     = string("IPv6");
        }
        inet_ntop(dst_info->ai_family, addr, ip.value, INET6_ADDRSTRLEN);
        log_info("[NET] %s: %s", ipver.value, ip.value);
    }

    freeaddrinfo(dst_info);
}

internal void
net_start_listening(String port)
{
    N_WIN32_Command c = {.type = N_WIN32_CommandType_StartListening};
    memory_copy(c.port, port.value, port.length);
    _net_win32_command_enqueue(c);
}

internal void
net_stop_listening()
{
    N_WIN32_Command c = {.type = N_WIN32_CommandType_StopListening};
    _net_win32_command_enqueue(c);
}

internal bool32
net_server_start()
{
    N_WIN32_Command c = {.type = N_WIN32_CommandType_StartListening};
    _net_win32_command_enqueue(c);
    return true;
}

internal void
net_client_connect(String address, String port)
{
    N_WIN32_Command c = {.type = N_WIN32_CommandType_ClientConnect};
    memory_copy(c.address, address.value, address.length);
    memory_copy(c.port, port.value, port.length);
    _net_win32_command_enqueue(c);
}

internal void
net_client_disconnect()
{
    N_WIN32_Command c = {.type = N_WIN32_CommandType_ClientDisconnect};
    _net_win32_command_enqueue(c);
}

internal void
net_send(uint64 connection_id, uint8* buffer, uint64 length)
{
    N_WIN32_Command c = {.type = N_WIN32_CommandType_Send, .connection_id = connection_id};
    xassert(length < array_count(c.data));
    memory_copy(c.data, buffer, length);
    c.data_length = (int32)length;
    _net_win32_command_enqueue(c);
}

/** Implementation Specific */
internal void
_net_win32_log_wsa_error_message()
{
    LPVOID lpMsgBuf;

    int32 err = WSAGetLastError();
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   (LPSTR)&lpMsgBuf, 0, NULL);

    log_error("[NET] WSA Error %lu: %s", err, (char*)lpMsgBuf);

    LocalFree(lpMsgBuf);
}

internal void
_net_win32_network_main_thread(void* data)
{
    os_thread_name_set(string("network_main_thread"));

    (void)data;

    NET_WIN32_CommandBuffer* cmd_buffer = _n_win32_command_buffer;

    for (;;)
    {
        os_rw_mutex_take_w(cmd_buffer->rw_mutex);
        while (interlocked_uint64_read(&cmd_buffer->command_count) == 0)
        {
            os_condition_variable_wait_rw_w(cmd_buffer->cv, cmd_buffer->rw_mutex, UINT64_MAX);
        }
        os_rw_mutex_drop_w(cmd_buffer->rw_mutex);

        for (;;)
        {
            // SPEED(selim): Test if `head & mask` trick for fast module actually improves speed (needs power of 2 buffer size and `mask = capacity - 1`).
            N_WIN32_Command* slot = &_n_win32_command_buffer->commands[_n_win32_command_buffer->head % _N_WIN32_COMMAND_BUFFER_SIZE];
            if (!interlocked_uint32_read(&slot->is_ready))
            {
                bool32 expected = 0;
                os_wait_on_address(&slot->is_ready, &expected, sizeof(expected), -1);
                continue;
            }

            log_info("[NET] Command received %s.", n_win32_command_names[slot->type].value);
            _net_win32_command_process(slot);
            slot->is_ready = false;
            log_info("[NET] Command processed %s.", n_win32_command_names[slot->type].value);

            interlocked_uint64_inc(&_n_win32_command_buffer->head);
            if (interlocked_uint64_add(&_n_win32_command_buffer->command_count, -1) == 0)
                break;
        }
    }
}

internal void
_net_win32_command_process(N_WIN32_Command* command)
{
    switch (command->type)
    {
    case N_WIN32_CommandType_StartListening:
    {
        if (interlocked_uint32_read(&_n_win32_ctx->network_state) == NET_NetworkStateType_Listening)
        {
            log_info("[NET] Already listening.");
            return;
        }

        struct addrinfo *address_info = NULL, hints;
        memory_zero(&hints, sizeof(hints));

        hints.ai_family   = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags    = AI_PASSIVE;

        int32 w32_result = getaddrinfo(NULL, (char*)command->port, &hints, &address_info);
        if (w32_result != 0)
        {
            _net_win32_log_wsa_error_message();
            log_error("[NET] `getaddrinfo` failed for server");
            return;
        }

        SOCKET           listening_socket = INVALID_SOCKET;
        struct addrinfo* current_address  = address_info;
        for (; current_address != 0; current_address = current_address->ai_next)
        {
            listening_socket = socket(address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol);
            if (listening_socket == INVALID_SOCKET)
            {
                continue;
            }

            w32_result = bind(listening_socket, current_address->ai_addr, (int)current_address->ai_addrlen);
            if (w32_result == SOCKET_ERROR)
            {
                _net_win32_log_wsa_error_message();
                log_error("[NET] Bind failed");
                closesocket(listening_socket);
                listening_socket = INVALID_SOCKET;

                continue;
            }

            break;
        }

        freeaddrinfo(address_info);

        if (listening_socket == INVALID_SOCKET)
        {
            _net_win32_log_wsa_error_message();
            log_error("[NET] Couldn't connect to given address");
            return;
        }

        if (listen(listening_socket, SOMAXCONN) == SOCKET_ERROR)
        {
            _net_win32_log_wsa_error_message();
            log_error("[NET] Listen failed");
            closesocket(listening_socket);
            return;
        }

        N_WIN32_ReceiveThreadInfo* listen_thread_info = arena_push_struct_zero(_n_w32_perm_arena, N_WIN32_ReceiveThreadInfo);
        listen_thread_info->listen_socket             = listening_socket;
        os_thread_launch(_net_win32_listen_thread, listen_thread_info);
    }
    break;
    case N_WIN32_CommandType_StopListening:
    {
        if (!_net_win32_close_socket(_n_win32_ctx->listen_socket))
        {
            return;
        }

        for (uint32 i = 0; i < NET_MAX_CLIENT_COUNT; i++)
        {
            N_WIN32_Connection* conn = &_n_win32_ctx->clients[i];
            if (interlocked_uint32_read(&conn->state) == NET_ConnectionStateType_Connected)
            {
                interlocked_uint32_exchange(&conn->should_disconnect, 1);
            }
        }

        _n_win32_ctx->network_state = NET_NetworkStateType_Stopped;
        interlocked_uint32_exchange(&_n_win32_ctx->network_state, NET_NetworkStateType_Disconnecting);
    }
    break;
    case N_WIN32_CommandType_NewConnection:
    {
        int32 connection_id = -1;

        for (int32 i = 0; i < NET_MAX_CLIENT_COUNT; i++)
        {
            if (_n_win32_ctx->clients[i].state == NET_ConnectionStateType_Uninitialized)
            {
                connection_id = i;
                break;
            }
        }

        if (connection_id == -1)
        {
            log_error("[NET] No available connection exists.");
            return;
        }

        N_WIN32_Connection* conn = &_n_win32_ctx->clients[connection_id];
        conn->socket             = command->socket_param;
        conn->state              = NET_ConnectionStateType_Connected;
        conn->connection_id      = (uint64)connection_id;

        N_WIN32_ClientThreadInfo* client_thread_info = arena_push_struct_zero(_n_w32_perm_arena, N_WIN32_ClientThreadInfo);
        client_thread_info->conn                     = conn;

        os_thread_launch(_net_win32_recv_thread, client_thread_info);
    }
    break;
    case N_WIN32_CommandType_ClientConnect:
    {
        if (_n_win32_ctx->network_state == NET_NetworkStateType_ConnectedToServer)
        {
            // TODO(selim): Maybe here we can disconnect and reconnect to the new address
            log_warn("[NET] Already connected to server");
            return;
        }

        struct addrinfo *address_info = NULL,
                        hints;

        memory_zero(&hints, sizeof(hints));
        hints.ai_family   = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        int32 w32_result = getaddrinfo((char*)command->address, (char*)command->port, &hints, &address_info);
        if (w32_result != 0)
        {
            log_error("[NET] Couldn't get address info: %d", address_info);
            return;
        }

        SOCKET connect_socket = INVALID_SOCKET;
        for (struct addrinfo* current_address = address_info; current_address != 0; current_address = current_address->ai_next)
        {
            connect_socket = socket(address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol);
            if (connect_socket == INVALID_SOCKET)
            {
                continue;
            }

            w32_result = connect(connect_socket, current_address->ai_addr, (int)current_address->ai_addrlen);
            if (w32_result == SOCKET_ERROR)
            {
                closesocket(connect_socket);
                connect_socket = INVALID_SOCKET;
                continue;
            }

            break;
        }

        freeaddrinfo(address_info);

        if (connect_socket == INVALID_SOCKET)
        {
            _net_win32_log_wsa_error_message();
            log_error("[NET] Couldn't connect to given address");
            return;
        }

        _n_win32_ctx->network_state = NET_NetworkStateType_ConnectedToServer;
        N_WIN32_Connection* conn    = &_n_win32_ctx->clients[0];
        conn->socket                = connect_socket;
        conn->state                 = NET_ConnectionStateType_Connected;
        conn->connection_id         = (uint64)0;

        N_WIN32_ClientThreadInfo* client_thread_info = arena_push_struct_zero(_n_w32_perm_arena, N_WIN32_ClientThreadInfo);
        client_thread_info->conn                     = conn;

        os_thread_launch(_net_win32_recv_thread, client_thread_info);

        log_info("[NET] Successfully connect to server");
    }
    break;
    case N_WIN32_CommandType_MessageReceived:
    {
        log_info("[NET] Message Received! Byte Length: %d, Bytes: %s", command->data_length, command->data);
        _n_win32_ctx->on_message(command->connection_id, command->data, command->data_length);
        break;
    }
    case N_WIN32_CommandType_Send:
    {
        N_WIN32_Connection* conn = &_n_win32_ctx->clients[command->connection_id];
        if (conn->state != NET_ConnectionStateType_Connected)
        {
            log_error("[Net] Connection not active. Id: %llu", command->connection_id);
            return;
        }

        int32 send_result = send(conn->socket, (char*)command->data, (int32)command->data_length, 0);
        if (send_result <= 0)
        {
            _net_win32_log_wsa_error_message();
            log_error("[NET] Send failed");
            return;
        }

        log_info("[NET] Sent data successfully, byte count: %d", send_result);
    }
    break;
    default:
    {
        log_error("[NET] Invalid Command Type: %d.", command->type);
    }
    break;
    }
}

internal void
_net_win32_listen_thread(void* data)
{
    os_thread_name_set(string("network_listen_thread"));

    N_WIN32_ReceiveThreadInfo* info = (N_WIN32_ReceiveThreadInfo*)data;
    log_info("[NET] Server started listening.");

    interlocked_uint32_exchange(&_n_win32_ctx->network_state, NET_NetworkStateType_Listening);
    while (interlocked_uint32_read(&_n_win32_ctx->network_state) == NET_NetworkStateType_Listening)
    {
        SOCKET client_socket = INVALID_SOCKET;

        client_socket = accept(info->listen_socket, NULL, NULL);
        log_info("[NET] Received connection request.");

        if (client_socket == INVALID_SOCKET)
        {
            _net_win32_log_wsa_error_message();
            log_error("[NET] Couldn't accept request");
            continue;
        }

        log_info("[NET] Accepted connection request.");

        N_WIN32_Command c = {.type = N_WIN32_CommandType_NewConnection, .socket_param = client_socket};
        _net_win32_command_enqueue(c);
    }

    interlocked_uint32_exchange(&_n_win32_ctx->network_state, NET_NetworkStateType_Stopped);
    log_info("[NET] Server stopped listening");
}

internal void
_net_win32_recv_thread(void* data)
{
    os_thread_name_set(string("network_recv_thread"));

    N_WIN32_ClientThreadInfo* info = (N_WIN32_ClientThreadInfo*)data;
    N_WIN32_Connection*       conn = info->conn;

    log_info("[NET] Launched receive thread. Connection Id: %d", conn->connection_id);

    while (!interlocked_uint32_read(&conn->should_disconnect))
    {
        N_WIN32_Command c           = {.type = N_WIN32_CommandType_MessageReceived};
        int32           data_length = recv(conn->socket, (char*)c.data, array_count(c.data), 0);

        if (data_length > 0)
        {
            log_info("[NET] Bytes received: %d", data_length);
            c.connection_id = conn->connection_id;
            c.data_length   = (uint64)data_length;
            _net_win32_command_enqueue(c);
        }
        else if (data_length == 0)
        {
            log_info("[NET] Received 0 length.");
            break;
        }
        else
        {
            _net_win32_log_wsa_error_message();
            log_error("[NET] `recv` failed");
            break;
        }
    };

    log_info("[NET] Connection closing.");
    _net_win32_close_socket(conn->socket);
    conn->socket = INVALID_SOCKET;
    interlocked_uint32_exchange(&conn->should_disconnect, 0);
    interlocked_uint32_exchange(&conn->state, NET_ConnectionStateType_Uninitialized);
    interlocked_uint32_dec(&_n_win32_ctx->client_count);
}

/** command bufer */
internal void
_net_win32_command_enqueue(N_WIN32_Command command)
{
    uint64           index = interlocked_uint64_exchange_add(&_n_win32_command_buffer->tail, 1);
    N_WIN32_Command* slot  = &_n_win32_command_buffer->commands[index % _N_WIN32_COMMAND_BUFFER_SIZE];
    memory_copy_struct(slot, &command);
    uint64 command_count = interlocked_uint64_exchange_add(&_n_win32_command_buffer->command_count, 1);
    interlocked_uint32_exchange(&slot->is_ready, 1);

    if (command_count == 0)
    {
        os_rw_mutex_take_w(_n_win32_command_buffer->rw_mutex);
        os_condition_variable_signal(_n_win32_command_buffer->cv);
        os_rw_mutex_drop_w(_n_win32_command_buffer->rw_mutex);
    }
}

internal bool32
_net_win32_close_socket(SOCKET socket)
{
    int32 w32_result = shutdown(socket, SD_SEND);
    if (w32_result != 0)
    {
        _net_win32_log_wsa_error_message();
        log_error("[NET] Couldn't shutdown socket");
        return false;
    }

    w32_result = closesocket(socket);
    if (w32_result != 0)
    {
        _net_win32_log_wsa_error_message();
        log_error("[NET] Couldn't close socket");
        return false;
    }

    return true;
}