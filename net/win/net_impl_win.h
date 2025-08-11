#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "../net_core.h"

global read_only Arena* _n_w32_perm_arena;
global read_only Arena* _n_w32_frame_arena;

typedef enum
{
    N_WIN32_EntityType_Undefined = 0,
    N_WIN32_EntityType_Socket
} N_WIN32_EntityType;

typedef struct
{
    N_WIN32_EntityType type;
    SOCKET             socket;
} N_WIN32_Entity;

typedef struct
{
    N_Server* server;
} N_WIN32_ServerThreadInfo;

typedef struct
{
    SOCKET client_socket;
} N_WIN32_ClientThreadInfo;

internal N_Handle _net_win32_connect(String address, String port);
internal bool32   _net_win32_close_socket(N_Handle socket_handle);
internal void     _net_win32_log_wsa_error_message();
internal void     _net_win32_listen_thread(void* data);
internal void     _net_win32_server_connection_thread(void* data);