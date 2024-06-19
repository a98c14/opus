#pragma once
#include "net_core_win32.h"

// NOTE(selim): make sure to link against ws2_32.lib for winsock 2
internal void
net_init()
{
    WSADATA wsa_data;

    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
    {
        log_error("failed WSAStartup during net_init");
        return;
    }

    if (LOBYTE(wsa_data.wVersion) != 2 || HIBYTE(wsa_data.wVersion) != 2)
    {
        log_error("version 2.2 of winsock is not available.");
        WSACleanup();
        return;
    }
}

internal String
net_get_ip(Arena* arena)
{
    String          result = string("");
    struct addrinfo hints  = {0};
    hints.ai_family        = AF_INET;     // IPv4 or IPv6
    hints.ai_socktype      = SOCK_STREAM; // TCP
    hints.ai_flags         = AI_PASSIVE;

    struct addrinfo* dst_info;
    int32            status = getaddrinfo(0, "5055", &hints, &dst_info);
    if (status != 0)
    {
        log_error("couldn't get address info for host, status: %d", status);
        return result;
    }

    String ip = string_new(arena, INET_ADDRSTRLEN);
    inet_ntop(dst_info->ai_family, (struct sockaddr_in*)dst_info->ai_addr, ip.value, INET_ADDRSTRLEN);
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
        log_error("couldn't get address info for, %s:%s, status: %d", address.value, port.value, status);
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

internal String
net_sockaddr_to_ip(Arena* arena, int32 family, struct sockaddr* info)
{
    String ip = string_new(arena, INET6_ADDRSTRLEN);
    void*  addr;
    if (family == AF_INET)
    {
        struct sockaddr_in* ipv4 = (struct sockaddr_in*)info;
        addr                     = &(ipv4->sin_addr);
    }
    else
    {
        struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)info;
        addr                      = &(ipv6->sin6_addr);
    }
    inet_ntop(family, addr, ip.value, INET6_ADDRSTRLEN);
    return ip;
}

internal N_Socket*
net_listen(Arena* arena, String port)
{
    N_Socket* result = arena_push_struct_zero(arena, N_Socket);

    struct addrinfo hints = {0};
    hints.ai_family       = AF_INET;
    hints.ai_socktype     = SOCK_STREAM; // TCP
    hints.ai_protocol     = IPPROTO_TCP;
    hints.ai_flags        = AI_PASSIVE;

    struct addrinfo* server_addr;
    int32            status = getaddrinfo(NULL, port.value, &hints, &server_addr);
    if (status != 0)
    {
        log_error("couldn't get address info for port %s, status: %d, error: %s", port.value, status, gai_strerrorA(status));
        return result;
    }

    struct addrinfo* node;
    for (node = server_addr; node != NULL; node = server_addr->ai_next)
    {
        result->descriptor = socket(node->ai_family, node->ai_socktype, node->ai_protocol);
        if (result->descriptor < 0)
        {
            log_error("couldn't create socket, %d", WSAGetLastError());
            continue;
        }

        // TODO(selim): might need to set `SO_REUSEADDR` here
        status = bind(result->descriptor, node->ai_addr, node->ai_addrlen);
        if (status != 0)
        {
            log_error("couldn't bind socket, status %d, error code: %d", status, WSAGetLastError());
            continue;
        }

        log_info("successfully created socket");
        break;
    }
    freeaddrinfo(server_addr);

    if (node == NULL)
    {
        log_error("server failed to bind");
        closesocket(result->descriptor);
        return result;
    }

    status = listen(result->descriptor, SOMAXCONN);
    if (status < 0)
    {
        log_error("encountered an error while trying to listen, %d", WSAGetLastError());
        return result;
    }

    result->state = N_SocketState_Ready;
    return result;
}

internal N_Socket*
net_connect(Arena* arena, String address, String port)
{
    N_Socket* result = arena_push_struct_zero(arena, N_Socket);

    struct addrinfo hints = {0};
    hints.ai_family       = AF_UNSPEC;   // IPv4 or IPv6
    hints.ai_socktype     = SOCK_STREAM; // TCP

    struct addrinfo* server_addr;
    int32            status = getaddrinfo(address.value, port.value, &hints, &server_addr);
    if (status != 0)
    {
        log_error("couldn't get address info for port %s, status: %d, error: %s", port.value, status, gai_strerrorA(status));
        return result;
    }

    struct addrinfo* node;
    for (node = server_addr; node != NULL; node = server_addr->ai_next)
    {
        result->descriptor = socket(node->ai_family, node->ai_socktype, node->ai_protocol);
        if (result->descriptor < 0)
        {
            log_error("couldn't create socket, %d", WSAGetLastError());
            continue;
        }

        status = connect(result->descriptor, node->ai_addr, node->ai_addrlen);
        if (status != 0)
        {
            closesocket(result->descriptor);
            log_error("couldn't connect to destination, status %d, error: %d", status, WSAGetLastError());
            continue;
        }

        log_info("successfully created socket");
        break;
    }

    if (node == NULL)
    {
        log_error("client: failed to connect");
        closesocket(result->descriptor);
        return result;
    }

    result->state = N_SocketState_Connected;
    String dst_ip = net_sockaddr_to_ip(arena, node->ai_family, node->ai_addr);
    printf("client: connected to %s\n", dst_ip.value);
    freeaddrinfo(server_addr);
    result->state = N_SocketState_Connected;
    return result;
}

internal void
net_socket_close(N_Socket* socket)
{
    closesocket(socket->descriptor);
}

internal void
net_cleanup()
{
    WSACleanup();
}

internal N_Socket*
net_accept(Arena* arena, N_Socket* socket)
{
    ArenaTemp               temp = scratch_begin(0, 0);
    struct sockaddr_storage incoming;
    socklen_t               addr_size = sizeof(incoming);

    N_Socket* conn   = arena_push_struct_zero(arena, N_Socket);
    conn->descriptor = accept(socket->descriptor, (struct sockaddr*)&incoming, &addr_size);
    if (conn->descriptor == -1)
    {
        log_error("couldn't accept incoming connection, %d", WSAGetLastError());
    }

    String incoming_ip = net_sockaddr_to_ip(temp.arena, incoming.ss_family, (struct sockaddr*)&incoming);
    printf("server: got connection from %s\n", incoming_ip.value);

    conn->state = N_SocketState_Connected;

    scratch_end(temp);
    return conn;
}

internal void
net_send(N_Socket* socket, void* bytes, uint64 length)
{
    int32 status = send(socket->descriptor, bytes, length, 0);
    if (status == -1)
    {
        log_error("couldn't send message, %d", WSAGetLastError());
    }
}

internal void*
net_receive(Arena* arena, N_Socket* socket)
{
    uint64 initial_pos = arena->pos;
    void*  buffer      = arena_push(arena, N_MAX_BUFFER_SIZE);

    int64 received = recv(socket->descriptor, (char*)buffer, N_MAX_BUFFER_SIZE - 1, 0);
    if (received == -1)
    {
        log_error("error while trying to receive incoming bytes, %d", WSAGetLastError());
    }
    arena_pop_to(arena, initial_pos + received);
    return buffer;
}
