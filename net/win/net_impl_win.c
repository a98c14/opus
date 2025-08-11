#include "net_impl_win.h"

// NOTE(selim): make sure to link against ws2_32.lib for winsock 2
internal void
net_init(void)
{
    WSADATA wsa_data;

    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
    {
        _net_win32_log_wsa_error_message();
        log_error("failed WSAStartup during net_init");
        return;
    }

    if (LOBYTE(wsa_data.wVersion) != 2 || HIBYTE(wsa_data.wVersion) != 2)
    {
        _net_win32_log_wsa_error_message();
        log_error("version 2.2 of winsock is not available.");
        WSACleanup();
        return;
    }

    _n_w32_perm_arena  = arena_new_reserve(mb(8));
    _n_w32_frame_arena = arena_new_reserve(mb(8));
}

internal void
net_cleanup()
{
    if (WSACleanup() != 0)
    {
        _net_win32_log_wsa_error_message();
        log_error("failed WSACleanup");
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
        log_error("Couldn't get address info for, %s:%s, status: %d", address.value, port.value, status);
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
        log_info("%s: %s", ipver.value, ip.value);
    }

    freeaddrinfo(dst_info);
}

internal N_Server*
net_server_new(String port)
{
    N_Server* result = arena_push_struct_zero(_n_w32_perm_arena, N_Server);

    result->state = N_ServerStateType_Ready;
    result->port  = port;
    result->mutex = os_mutex_alloc();

    for (uint32 i = 0; i < N_MAX_CLIENT_COUNT; i++)
    {
        // TODO(selim): add alloc/release entity functions with locks
        N_WIN32_Entity* client_entity = arena_push_struct_zero(_n_w32_perm_arena, N_WIN32_Entity);
        client_entity->type           = N_WIN32_EntityType_Socket;

        result->clients[i].socket = (N_Handle){int_from_ptr(client_entity)};
    }

    return result;
}

static void
log_gai_error(int rc)
{
    const char* msg = gai_strerrorA(rc);
    fprintf(stderr, "getaddrinfo failed (%d): %s\n", rc, msg ? msg : "(unknown)");
}

internal bool32
net_server_start(N_Server* server)
{
    if (!server || server->state != N_ServerStateType_Ready)
    {
        log_error("Invalid server state.");
        return 0;
    }

    struct addrinfo *address_info = NULL, hints;

    memory_zero(&hints, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags    = AI_PASSIVE;

    int32 w32_result = getaddrinfo(NULL, server->port.value, &hints, &address_info);
    if (w32_result != 0)
    {
        _net_win32_log_wsa_error_message();
        log_error("`getaddrinfo` failed for server");
        return 0;
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
            log_error("Bind failed");
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
        log_error("Couldn't connect to given address");
        return 0;
    }

    if (listen(listening_socket, SOMAXCONN) == SOCKET_ERROR)
    {
        _net_win32_log_wsa_error_message();
        log_error("Listen failed");
        closesocket(listening_socket);
        return 0;
    }

    N_Handle handle = {0};

    N_WIN32_Entity* socket_entity = arena_push_struct_zero(_n_w32_perm_arena, N_WIN32_Entity);
    socket_entity->socket         = listening_socket;
    socket_entity->type           = N_WIN32_EntityType_Socket;
    handle.v                      = int_from_ptr(socket_entity);

    server->listening_socket = handle;
    server->state            = N_ServerStateType_Listening;

    N_WIN32_ServerThreadInfo* server_thread_info = arena_push_struct_zero(_n_w32_perm_arena, N_WIN32_ServerThreadInfo);
    server_thread_info->server                   = server;
    os_thread_launch(_net_win32_listen_thread, server_thread_info);

    return true;
}

internal bool32
net_server_stop(N_Server* server)
{
    if (server->state != N_ServerStateType_Listening)
    {
        return true;
    }

    os_mutex_take(server->mutex);
    _net_win32_close_socket(server->listening_socket);
    for (uint32 i = 0; i < server->client_count; i++)
    {
        N_Connection* connection = &server->clients[i];
        if (connection->state == N_ConnectionStateType_Connected)
        {
            _net_win32_close_socket(connection->socket);
            connection->state = N_ConnectionStateType_Uninitialized;
        }
    }

    server->client_count     = 0;
    server->state            = N_ServerStateType_Disconnected;
    server->listening_socket = n_handle_zero;
    os_mutex_drop(server->mutex);
}

internal N_Client*
net_client_new()
{

    N_Client* result = arena_push_struct_zero(_n_w32_perm_arena, N_Client);
    return result;
}

internal bool32
net_client_connect(N_Client* client, String address, String port)
{
    N_Handle socket_handle = _net_win32_connect(address, port);
    if (socket_handle.v == 0)
    {
        return false;
    }

    client->conn.socket = socket_handle;
    client->conn.state  = N_ConnectionStateType_Connected;
    return true;
}

internal bool32
net_client_disconnect(N_Client* client)
{
    if (client->conn.state != N_ConnectionStateType_Connected)
    {
        return true;
    }

    return _net_win32_close_socket(client->conn.socket);
}

internal bool32
net_send(N_Connection socket, Buffer buffer)
{
    N_WIN32_Entity* entity = ptr_from_int(socket.socket.v);

    int32 send_result = send(entity->socket, buffer.data, (int32)buffer.size, 0);
    if (send_result == SOCKET_ERROR)
    {
        _net_win32_log_wsa_error_message();
        log_error("Send failed");
        return false;
    }
    log_info("Sent data successfully, byte count: %d", send_result);

    return true;
}

/** Implementation Specific */
internal N_Handle
_net_win32_connect(String address, String port)
{
    log_info("Trying to connect to server.");
    N_Handle handle = {0};

    struct addrinfo *address_info = NULL,
                    hints;

    memory_zero(&hints, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int32 w32_result = getaddrinfo(address.value, port.value, &hints, &address_info);
    if (w32_result != 0)
    {
        log_error("Couldn't get address info: %d", address_info);
        return handle;
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
        log_error("Couldn't connect to given address");
        return handle;
    }

    N_WIN32_Entity* socket_entity = arena_push_struct_zero(_n_w32_perm_arena, N_WIN32_Entity);
    socket_entity->socket         = connect_socket;
    socket_entity->type           = N_WIN32_EntityType_Socket;
    handle.v                      = int_from_ptr(socket_entity);

    log_info("Successfully connect to server");

    return handle;
}

internal bool32
_net_win32_close_socket(N_Handle socket_handle)
{
    N_WIN32_Entity* entity = ptr_from_int(socket_handle.v);
    if (entity->type != N_WIN32_EntityType_Socket)
    {
        log_error("Client socket type is invalid! Should have never happened");
        return true;
    }

    int32 w32_result = shutdown(entity->socket, SD_SEND);
    if (w32_result != 0)
    {
        _net_win32_log_wsa_error_message();
        log_error("Couldn't shutdown socket");
        return false;
    }

    w32_result = closesocket(entity->socket);
    if (w32_result != 0)
    {
        _net_win32_log_wsa_error_message();
        log_error("Couldn't close socket");
        return false;
    }

    return true;
}

internal void
_net_win32_log_wsa_error_message()
{
    LPVOID lpMsgBuf;

    int32 err = WSAGetLastError();
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPSTR)&lpMsgBuf,
        0,
        NULL);

    log_error("WSA Error %lu: %s", err, (char*)lpMsgBuf);

    LocalFree(lpMsgBuf);
}

internal void
_net_win32_listen_thread(void* data)
{
    N_WIN32_ServerThreadInfo* info = (N_WIN32_ServerThreadInfo*)data;
    log_info("Server started listening");

    N_Server*       server        = info->server;
    N_WIN32_Entity* listen_socket = ptr_from_int(server->listening_socket.v);

    while (true)
    {
        SOCKET client_socket = INVALID_SOCKET;

        client_socket = accept(listen_socket->socket, NULL, NULL);
        log_info("Received connection request.");

        if (client_socket == INVALID_SOCKET)
        {
            _net_win32_log_wsa_error_message();
            log_error("Couldn't accept request");
            continue;
        }

        log_info("Accepted connection request.");

        xassert(server->client_count < N_MAX_CLIENT_COUNT);
        os_mutex_take(server->mutex);

        N_Connection* conn = &server->clients[server->client_count++];
        conn->state        = N_ConnectionStateType_Connected;

        N_WIN32_Entity* client_socket_entity = ptr_from_int(conn->socket.v);
        client_socket_entity->socket         = client_socket;

        N_WIN32_ClientThreadInfo* client_thread_info = arena_push_struct_zero(_n_w32_perm_arena, N_WIN32_ClientThreadInfo);
        client_thread_info->client_socket            = client_socket;
        os_thread_launch(_net_win32_server_connection_thread, client_thread_info);

        os_mutex_drop(server->mutex);
    }

    log_info("Server stopped listening");
}

internal void
_net_win32_server_connection_thread(void* data)
{
    N_WIN32_ClientThreadInfo* info = (N_WIN32_ClientThreadInfo*)data;

    char  receive_buffer[256] = {0};
    int32 receive_result      = 0;
    do
    {
        receive_result = recv(info->client_socket, receive_buffer, 256, 0);

        if (receive_result > 0)
        {
            log_info("Bytes received: %d", receive_result);
        }
        else if (receive_result == 0)
        {
            log_info("Connection closing.");
        }
        else
        {
            _net_win32_log_wsa_error_message();
            log_error("`recv` failed");
            // closesocket(info->client_socket);
        }
    } while (receive_result > 0);
}