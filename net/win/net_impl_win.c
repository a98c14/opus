#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "../net_core.h"

// NOTE(selim): make sure to link against ws2_32.lib for winsock 2
internal void
net_init(void)
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

internal void
net_socket_close(void)
{

    // closesocket(SOCKET s)
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

internal N_Socket*
net_listen(Arena* arena, String port)
{
    N_Socket* result = arena_push_struct_zero(arena, N_Socket);

    struct addrinfo hints = {0};
    hints.ai_family       = AF_UNSPEC;   // IPv4 or IPv6
    hints.ai_socktype     = SOCK_STREAM; // TCP
    hints.ai_flags        = AI_PASSIVE;

    struct addrinfo* src_addr;
    int32            status = getaddrinfo(NULL, port.value, &hints, &src_addr);
    if (status != 0)
    {
        log_error("couldn't get address info for port %s, status: %d", port.value, status);
        return result;
    }

    result->descriptor = socket(src_addr->ai_family, src_addr->ai_socktype, src_addr->ai_protocol);
    if (result->descriptor < 0)
    {
        log_error("couldn't create socket, %d", WSAGetLastError());
        return result;
    }

    status = bind(result->descriptor, src_addr->ai_addr, src_addr->ai_addrlen);
    if (status != 0)
    {
        log_error("couldn't bind socket, status %d, error code: %d", status, WSAGetLastError());
        return result;
    }

    status = listen(result->descriptor, N_BACKLOG);
    if (result->descriptor < 0)
    {
        log_error("encountered an error while trying to listen, %d", WSAGetLastError());
        return result;
    }

    return result;
}

internal void
net_accept(N_Socket* socket)
{
    struct sockaddr_storage incoming;
    socklen_t               addr_size = sizeof(incoming);

    int32 conn_sd = accept(socket->descriptor, (struct sockaddr*)&incoming, &addr_size);
    // TODO(selim):
}

internal void
net_send(N_Socket* socket, void* bytes, uint64 length)
{
    // sendto(socket->descriptor, bytes, length, 0, , sizeof(struct sockaddr_storage));
}

internal void
net_connect(Arena* arena, String address, String port)
{
    struct addrinfo hints = {0};
    hints.ai_family       = AF_UNSPEC;   // IPv4 or IPv6
    hints.ai_socktype     = SOCK_STREAM; // TCP
    hints.ai_flags        = AI_PASSIVE;

    struct addrinfo* dst_addr;
    int32            status = getaddrinfo(address.value, port.value, &hints, &dst_addr);
    if (status != 0)
    {
        log_error("couldn't get address info for, %s:%s, status: %d", address.value, port.value, status);
        return;
    }

    struct addrinfo* src_addr;
    status = getaddrinfo(NULL, "80", &hints, &src_addr);
    if (status != 0)
    {
        log_error("couldn't get address info for, %s:%s, status: %d", address.value, port.value, status);
        return;
    }

    // TODO(selim): we need to validate the dst_info (like in `net_host_to_ip`). not sure
    // how to determine which node in linked list is good though.
    // int32 sockfd = socket(src_addr->ai_family, src_addr->ai_socktype, src_addr->ai_protocol);
    // if (sockfd < 0)
    // {
    //     log_error("couldn't create socket, %d", WSAGetLastError());
    //     return;
    // }

    // status = bind(sockfd, src_addr->ai_addr, src_addr->ai_addrlen);
    // if (status != 0)
    // {
    //     log_error("couldn't bind socket, status %d, error code: %d", status, WSAGetLastError());
    //     return;
    // }

    int32 sockfd = socket(dst_addr->ai_family, dst_addr->ai_socktype, dst_addr->ai_protocol);
    if (sockfd < 0)
    {
        log_error("couldn't create socket, %d", WSAGetLastError());
        return;
    }

    status = connect(sockfd, dst_addr->ai_addr, dst_addr->ai_addrlen);
    if (status != 0)
    {
        log_error("couldn't connect to %s:%s, error: %s", address.value, port.value, WSAGetLastError());
    }
    log_info("successfully connected to destination");
}