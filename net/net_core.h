#pragma once
#include <base.h>

#define N_BACKLOG 10

typedef struct
{
    int32 descriptor;
} N_Socket;

internal void net_init();
internal void net_socket_close();

internal String    net_get_ip(Arena* arena);
internal void      net_host_to_ip(Arena* arena, String address, String port);
internal void      net_connect(Arena* arena, String address, String port);
internal N_Socket* net_listen(Arena* arena, String port);
internal void      net_accept(N_Socket* socket);