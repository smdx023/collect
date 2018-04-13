#include "collect_service.h"

#include "collect.h"
#include <sp.h>
#include <signal.h>

static void session_close(session_t *session);

int collect_service_init()
{
    app_context_t *ctx = app_context();

    const char *address = sp_ini_get_string(ctx->ini, "collect", "address");
    int port = sp_ini_get_int(ctx->ini, "collect", "port");

    int sock = sp_socket(AF_INET, SOCK_STREAM , 0);
    sp_socket_reuseable(sock);

    struct sockaddr_in addr;
    sp_bzero(&addr, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(address);

    if (sp_socket_bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        app_log_error("bind failed");
        return -1;
    }

    if (sp_socket_listen(sock, 5) < 0)
    {
        app_log_error("listen failed");
        return -1;
    }

    ctx->reactor = sp_reactor_new(NULL);

    ctx->collect_ev = sp_reactor_attach(ctx->reactor, sock, handle_connect, NULL);

    ctx->collect_sock = sock;

    app_log_warn("collect service listen on %s:%d", address, port);

    signal(SIGPIPE, SIG_IGN);
    
    return 0;
}

int collect_service_run()
{
    app_context_t *ctx = app_context();

    sp_reactor_run(ctx->reactor, 0);
    
    return 0;
}

void collect_service_fini()
{

}

void handle_connect(int sock, void *arg)
{
    struct sockaddr_in addr;
    size_t addr_size = sizeof(addr);
    sp_bzero(&addr, sizeof(struct sockaddr_in));

    int s = sp_socket_accept(sock, (struct sockaddr *)&addr, &addr_size);
    if (s < 0)
    {
        app_log_error("accept failed");
        return;
    }

    app_context_t *ctx = app_context();

    session_t *session = sp_calloc(1, sizeof(session_t));
    session->sock = s;
    session->ev = sp_reactor_attach(ctx->reactor, s, handle_read, session);
    session->protocol_type = PROTOCOL_TYPE_UNKNOWN;

    app_log_debug("%s connected, sock = %d", inet_ntoa(addr.sin_addr), s);
}

void handle_read(int sock, void *arg)
{
    session_t *session = (session_t *)arg;

    int readable = sp_socket_readable(sock);

    if (readable <= 0)
    {
        app_log_debug("sock = %d disconnected", session->sock);
        session_close(session);
        return;
    }
    else
    {
        char *buffer = sp_malloc(readable);
        int length = sp_socket_read(sock, buffer, readable);

        if (session->protocol_type == PROTOCOL_TYPE_UNKNOWN)
        {
            
            session->protocol_type = protocol_try_parse(buffer, length);
        }

        protocol_parser_t *parser = protocol_parser(session->protocol_type);

        parser->handle_input(buffer, length, session);
    }
}

static void session_close(session_t *session)
{
    sp_return_if_fail(session);

    sp_reactor_detach(session->ev);

    sp_socket_close(session->sock);

    sp_free(session);
}
