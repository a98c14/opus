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
    N_NetworkStateType network_state;

    SOCKET listen_socket;
    String listen_port;

    uint32             client_count;
    N_WIN32_Connection clients[_N_WIN32_MAX_CLIENT_COUNT];
} N_WIN32_Context;

typedef enum
{
    N_WIN32_CommandType_NewConnection,
    N_WIN32_CommandType_DisconnectConnection,
    N_WIN32_CommandType_MessageReceived,
    N_WIN32_CommandType_Send,
    N_WIN32_CommandType_StartListening,
    N_WIN32_CommandType_StopListening,
    N_WIN32_CommandType_ClientConnect,
    N_WIN32_CommandType_ClientDisconnect,
    N_WIN32_CommandType_COUNT,
} N_WIN32_CommandType;

global String n_win32_command_names[N_WIN32_CommandType_COUNT] = {
    string_comp("NewConnection"),
    string_comp("DisconnectConnection"),
    string_comp("MessageReceived"),
    string_comp("MessageSend"),
    string_comp("StartListening"),
    string_comp("StopListening"),
    string_comp("ClientConnect"),
    string_comp("ClientDisconnect")};

typedef struct
{
    N_WIN32_CommandType type;

    /** params */
    bool32 is_ready;
    int32  data_length;
    uint8  data[256];

    SOCKET socket_param;
    uint64 connection_id;

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
    N_WIN32_Connection* conn;
} N_WIN32_ClientThreadInfo;

internal bool32 _net_win32_close_socket(SOCKET socket);
internal void   _net_win32_log_wsa_error_message();

internal void _net_win32_network_main_thread(void* data);
internal void _net_win32_listen_thread(void* data);
internal void _net_win32_recv_thread(void* data);

/** command bufer */
internal void _net_win32_command_enqueue(N_WIN32_Command command);
internal void _net_win32_command_process(N_WIN32_Command* command);