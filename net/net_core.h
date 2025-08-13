#pragma once
/**
 * NOTE(selim): make sure to link against ws2_32.lib for winsock 2
 * */

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
    N_ConnectionStateType_Disconnected  = 3,
    N_ConnectionStateType_COUNT
} N_ConnectionStateType;

typedef enum
{
    N_ServerStateType_Uninitialized = 0,
    N_ServerStateType_Ready         = 1,
    N_ServerStateType_Listening     = 2,
    N_ServerStateType_Stopped       = 3,
    N_ServerStateType_COUNT
} N_ServerStateType;

internal void net_init();
internal void net_cleanup();

internal void net_start_listening(String port);
internal void net_stop_listening();

internal void net_client_connect(String address, String port);
internal void net_client_disconnect();

/** Sends data to through given connection, 0 means client->server */
internal void net_send(uint64 connection_id, Buffer buffer);

/** Sends data to all connected clients */
internal void net_send_all(Buffer buffer);