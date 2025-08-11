#pragma once
#include "../base/base_inc.h"
#include "../os/os_inc.h"

#define N_BACKLOG          10
#define N_DEFAULT_PORT     "27015"
#define N_MAX_CLIENT_COUNT 128

typedef struct
{
    uint64 v;
} N_Handle;

global read_only N_Handle n_handle_zero = {0};

typedef enum
{
    N_ConnectionStateType_Uninitialized = 0,
    N_ConnectionStateType_Ready         = 1,
    N_ConnectionStateType_Connected     = 2,
    N_ConnectionStateType_COUNT
} N_ConnectionStateType;

typedef enum
{
    N_ServerStateType_Uninitialized = 0,
    N_ServerStateType_Ready         = 1,
    N_ServerStateType_Listening     = 2,
    N_ServerStateType_Disconnected  = 3,
    N_ServerStateType_COUNT
} N_ServerStateType;

typedef struct
{
    String                name;
    N_ConnectionStateType state;
    N_Handle              socket;
} N_Connection;

typedef struct
{
    N_ServerStateType state;
    String            port;
    N_Handle          listening_socket;

    uint32       client_count;
    N_Connection clients[N_MAX_CLIENT_COUNT];
    OS_Handle    mutex;
} N_Server;

typedef struct
{
    N_Connection conn;
} N_Client;

internal void net_init();
internal void net_cleanup();

internal N_Server* net_server_new(String port);

internal bool32 net_server_start(N_Server* server);
internal bool32 net_server_stop(N_Server* server);

internal N_Client* net_client_new();
internal bool32    net_client_connect(N_Client* client, String address, String port);
internal bool32    net_client_disconnect(N_Client* client);

internal bool32 net_send(N_Connection socket, Buffer buffer);