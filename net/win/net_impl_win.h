#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "../net_core.h"

global read_only Arena* _n_w32_perm_arena;
global read_only Arena* _n_w32_frame_arena;

#define _N_WIN32_COMMAND_BUFFER_SIZE 512
#define _N_WIN32_MAX_CLIENT_COUNT    64

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
    N_ConnectionStateType state;
    SOCKET                socket;
    uint64                connection_id;
    bool32                should_disconnect;
} N_WIN32_Connection;

typedef struct
{
    /** Server */
    bool32            listen_thread_should_quit;
    bool32            listen_thread_is_running;
    OS_Handle         listen_thread;
    N_ServerStateType listen_state;
    SOCKET            listen_socket;
    String            listen_port;

    uint32             client_count;
    N_WIN32_Connection clients[_N_WIN32_MAX_CLIENT_COUNT];

    /** Client */
    N_ConnectionStateType client_state;
    SOCKET                client_socket;

} N_WIN32_Context;

typedef enum
{
    N_WIN32_CommandType_NewConnection,
    N_WIN32_CommandType_DisconnectConnection,
    N_WIN32_CommandType_MessageReceived,
    N_WIN32_CommandType_MessageSend,
    N_WIN32_CommandType_StartListening,
    N_WIN32_CommandType_StopListening,
    N_WIN32_CommandType_ClientConnect,
    N_WIN32_CommandType_ClientDisconnect,
} N_WIN32_CommandType;

typedef struct
{
    N_WIN32_CommandType type;

    /** params */
    bool32 is_ready;
    SOCKET socket_param;
    uint64 data_length;
    uint8  data[256];

    uint8 address[32];
    uint8 port[4];
} N_WIN32_Command;

typedef struct
{
    OS_Handle rw_mutex;
    OS_Handle cv;

    uint64          command_count;
    uint64          head;
    uint64          tail;
    uint64          tail_reserved;
    N_WIN32_Command commands[_N_WIN32_COMMAND_BUFFER_SIZE];
} N_WIN32_CommandBuffer;

typedef struct
{
    SOCKET listen_socket;
} N_WIN32_ReceiveThreadInfo;

typedef struct
{
    uint64 connection_id;
    SOCKET client_socket;
} N_WIN32_ClientThreadInfo;

internal N_Handle _net_win32_connect(String address, String port);
internal bool32   _net_win32_close_socket(SOCKET socket);
internal void     _net_win32_log_wsa_error_message();

internal void _net_win32_network_main_thread(void* data);
internal void _net_win32_listen_thread(void* data);
internal void _net_win32_recv_thread(void* data);

/** command bufer */
internal void _net_win32_command_enqueue(N_WIN32_Command command);
internal void _net_win32_command_process(N_WIN32_Command* command);