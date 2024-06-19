#include "net_sample.h"

internal void
client_entry_point(void* data)
{
    /** init */
    os_thread_name_set(string("client_thread"));
    client_mutex = os_mutex_alloc();

    Arena*    perm_arena  = make_arena_reserve(mb(16));
    Arena*    frame_arena = make_arena_reserve(mb(16));
    ArenaTemp temp        = scratch_begin(0, 0);

    ClientOptions* options = (ClientOptions*)data;
    N_Socket*      socket  = net_connect(perm_arena, options->address, options->port);

    for (;;)
    {
        net_receive(frame_arena, socket);
        os_thread_sleep_ms(1000);
    }

    net_socket_close(socket);
    net_cleanup();
}