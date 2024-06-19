#pragma once
#include <base.h>

#define N_BACKLOG         10
#define N_MAX_BUFFER_SIZE 4096

typedef enum
{
    N_SocketState_Undefined,
    N_SocketState_Ready,
    N_SocketState_Disconnected,
    N_SocketState_Connected,
    N_SocketState_COUNT
} N_SocketState;

typedef struct
{
    N_SocketState state;
    int32         descriptor;
} N_Socket;

typedef struct
{
    uint64 v;
} N_Handle;

internal void net_init();

internal String net_get_ip(Arena* arena);
internal void   net_host_to_ip(Arena* arena, String address, String port);

// TODO(selim): should we use NET_Handle here?
internal N_Socket* net_connect(Arena* arena, String address, String port);
internal N_Socket* net_listen(Arena* arena, String port);
internal N_Socket* net_accept(Arena* arena, N_Socket* socket);
internal void      net_send(N_Socket* socket, void* bytes, uint64 length);
internal void*     net_receive(Arena* arena, N_Socket* socket);
internal void      net_socket_close(N_Socket* socket);
internal void      net_cleanup();