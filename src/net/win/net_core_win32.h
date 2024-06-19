#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "../net_core.h"

internal String net_sockaddr_to_ip(Arena* arena, int32 family, struct sockaddr* info);