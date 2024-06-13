#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>

#include "../net_core.h"

// NOTE(selim): make sure to link against ws2_32.lib for winsock 2
internal void
net_init()
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
net_socket_close()
{
    // closesocket(SOCKET s)
}