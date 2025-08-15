#pragma once
/**
 * NOTE(selim): make sure to link against ws2_32.lib for winsock 2
 * */

#include "../base/base_inc.h"
#include "../os/os_inc.h"

#define N_BACKLOG            10
#define N_DEFAULT_PORT       "27015"
#define NET_MAX_CLIENT_COUNT 128

typedef struct
{
    uint64 v;
} NET_Handle;

global read_only NET_Handle NET_Handle_zero = {0};

typedef enum
{
    NET_ConnectionStateType_Uninitialized = 0,
    NET_ConnectionStateType_Ready         = 1,
    NET_ConnectionStateType_Connected     = 2,
    NET_ConnectionStateType_Disconnected  = 3,
    NET_ConnectionStateType_COUNT
} NET_ConnectionStateType;

typedef enum
{
    NET_NetworkStateType_Uninitialized     = 0,
    NET_NetworkStateType_Ready             = 1,
    NET_NetworkStateType_Listening         = 2,
    NET_NetworkStateType_Disconnecting     = 3,
    NET_NetworkStateType_Stopped           = 4,
    NET_NetworkStateType_ConnectedToServer = 5,
    NET_NetworkStateType_COUNT
} NET_NetworkStateType;

typedef void NET_OnMessageEventHandlerType(uint64 connection_id, uint8* data, uint64 length);

internal void net_init(NET_OnMessageEventHandlerType* on_message);
internal void net_cleanup();

internal void net_start_listening(String port);
internal void net_stop_listening();

internal void net_client_connect(String address, String port);
internal void net_client_disconnect();

/** Sends data to through given connection, 0 means client->server */
internal void net_send(uint64 connection_id, uint8* buffer, uint64 length);

/** Sends data to all connected clients */
internal void net_send_all(Buffer buffer);