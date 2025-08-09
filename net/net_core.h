#pragma once
#include "../base/base_inc.h"

#define N_BACKLOG 10

typedef struct
{
    int32 descriptor;
} N_Socket;

typedef struct
{
    int32 t;
} N_Server;

internal void net_init();
internal void net_socket_close();

internal String    net_get_ip(Arena* arena);
internal void      net_host_to_ip(Arena* arena, String address, String port);
internal void      net_connect(Arena* arena, String address, String port);
internal N_Socket* net_listen(Arena* arena, String port);
internal void      net_accept(N_Socket* socket);

/** V2 */
internal N_Server net_server_new(Arena* arena, String port);
internal bool32   net_server_listen(N_Server* server);