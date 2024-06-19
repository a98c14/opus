#pragma once
#include <base.h>
#include <net/net_inc.h>
#include <os/os_inc.h>

typedef struct
{
    String name;
    String port;
} ServerOptions;

typedef struct
{
    String name;
    String address;
    String port;
} ClientOptions;

global OS_Handle server_mutex;
global OS_Handle client_mutex;