#include "net_sample.h"

internal void
server_entry_point(void* data)
{
    /** init */
    os_thread_name_set(string("server_thread"));
    server_mutex = os_mutex_alloc();

    Arena*    perm_arena  = make_arena_reserve(mb(16));
    Arena*    frame_arena = make_arena_reserve(mb(16));
    ArenaTemp temp        = scratch_begin(0, 0);

    // TODO(selim): should this be done in main thread?
    // net_init();
    String ip = net_get_ip(frame_arena);
    log_info("IPv4 address is: %s", ip.value);

    ServerOptions* options = (ServerOptions*)data;
    N_Socket*      socket  = net_listen(perm_arena, options->port);

    log_info("awaiting for connections");
    N_Socket* conn = net_accept(perm_arena, socket);
    log_info("accepted connection");
    net_send(conn, "Test Message", 13);

    for (;;)
    {
        os_thread_sleep_ms(100);
    }

    net_socket_close(socket);
    net_cleanup();
}