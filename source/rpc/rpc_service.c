#include "collect.h"

static sp_json_t* rpc_control(sp_jsonrpc_t *rpc);
static void task_cb(sp_jsonrpc_t *rpc);
static int addr_binded_sock(frame_376_address_t *addr);
static int control_buffer(byte* buffer, frame_376_address_t *addr, const char *method, sp_json_t *params);
static int control(int sock, frame_376_address_t *addr, const char *method, sp_json_t *params);


int rpc_server_new()
{
    app_context_t *ctx = app_context();

    const char *rpc_host = sp_ini_get_string(ctx->ini, "rpc", "address");
    int rpc_port = sp_ini_get_int(ctx->ini, "rpc", "port");

    app_log_warn("rpc host = %s, port = %d", rpc_host, rpc_port);
    ctx->rpc_server = sp_jsonrpc_server_new(rpc_host, rpc_port);
    sp_return_val_if_fail(ctx->rpc_server, -1);

    sp_jsonrpc_server_register(ctx->rpc_server, "control", rpc_control, NULL);

    return 0;
}

void rpc_server_run()
{
    app_context_t *ctx = app_context();

    sp_jsonrpc_server_run(ctx->rpc_server, SP_REACTOR_RUN_THREAD);
}

void rpc_server_stop()
{
    app_context_t *ctx = app_context();

    sp_jsonrpc_server_stop(ctx->rpc_server);
}

void rpc_server_free()
{
    app_context_t *ctx = app_context();

    sp_jsonrpc_server_free(ctx->rpc_server);
}

static sp_json_t* rpc_control(sp_jsonrpc_t *rpc)
{
    int code = -1;
    char *message = NULL;

    do
    {
        sp_break_if_fail(rpc && rpc->params);

        sp_break_if_fail(sp_json_array_size(rpc->params) == 1);

        sp_json_t *param = sp_json_array_item(rpc->params, 0);

        sp_json_t *payload = sp_json_parse(param->valuestring);

        sp_json_t *area = sp_json_object_item(payload, "area");
        sp_json_t *addr = sp_json_object_item(payload, "addr");
        sp_json_t *data = sp_json_object_item(payload, "data");

        sp_break_if_fail(area && addr && data);

        app_log_debug("area:%s, addr:%s", area->valuestring, addr->valuestring);

        int i;
        for (i = 0; i < sp_json_array_size(data); i++)
        {
            sp_json_t *pn_control = sp_json_array_item(data, i);
            sp_json_t *pn = sp_json_object_item(pn_control, "pn");
            sp_json_t *method = sp_json_object_item(pn_control, "method");
            sp_json_t *params = sp_json_object_item(pn_control, "params");

            app_log_debug("pn:%s, method:%s", pn->valuestring, method->valuestring);
        }

        task_t *task = sp_malloc(sizeof(task_t));
        rpc->priv = payload;
        task->cb = task_cb;
        task->arg = rpc;

        int a1 = atoi(area->valuestring);
        int a2 = atoi(addr->valuestring);

        frame_376_address_t address;
        int_2_bcd2(&address.a1, a1);
        address.a2 = a2;
        frame376_put_task(&address, task);

        return NULL; 

    } while(0);

    sp_json_t *result = sp_json_object_new();
    sp_json_object_add(result, "result", sp_json_int(code));
    message = rpc_error_message(code);
    if (message)
    {
        sp_json_object_add(result, "message", sp_json_string(message));
    }

    return result;
}

char *rpc_error_message(int err)
{
    char *message = NULL;

    switch (err)
    {
        case -1:
            message = "invalid params";
            break;
        case -2:
            message = "not implemented";
            break;
        case -3:
            message = "could not find device";
            break;
        case -4:
            message = "socket error";
            break;
        default:
            break;
    }

    return message;
}

static int addr_binded_sock(frame_376_address_t *addr)
{
    task_context_t *ctx = app_task_context();
    sp_return_val_if_fail(ctx, -1);

    unsigned int key = frame376_address_hash(addr);

    int sock = sp_hashtable_get(ctx->addr2sock, key);

    return sock;
}

static void task_cb(sp_jsonrpc_t *rpc)
{
    sp_json_t *payload = rpc->priv;
    
    sp_json_t *area = sp_json_object_item(payload, "area");
    sp_json_t *addr = sp_json_object_item(payload, "addr");
    sp_json_t *data = sp_json_object_item(payload, "data");

    sp_json_t *pn_control = sp_json_array_item(data, 0);
    sp_json_t *pn = sp_json_object_item(pn_control, "pn");
    sp_json_t *method = sp_json_object_item(pn_control, "method");
    sp_json_t *params = sp_json_object_item(pn_control, "params");

    int a1 = atoi(area->valuestring);
    int a2 = atoi(addr->valuestring);
    frame_376_address_t address;
    int_2_bcd2(&address.a1, a1);
    address.a2 = a2;
    address.pn = atoi(pn->valuestring);
    int sock = addr_binded_sock(&address);

    int code = -1;

    do
    {
        if (sock <= 0)
        {
            code = -3;
            app_log_error("addr(%d, %d) -> sock(%d)", address.a1, address.a2, sock);
            break;
        }

        code = control(sock, &address, method->valuestring, params);
    } while(0);

    sp_json_t *result = sp_json_object_new();
    sp_json_object_add(result, "result", sp_json_int(code));
    char *message = rpc_error_message(code);
    if (message)
    {
        sp_json_object_add(result, "message", sp_json_string(message));
    }

    sp_jsonrpc_session_reply_result(rpc->session, result, NULL);
    sp_json_free(result);

    sp_json_free(payload);
    sp_jsonrpc_session_close(rpc->session);
}

static int control(int sock, frame_376_address_t *addr, const char *method, sp_json_t *params)
{
    byte buffer[1024];

    int length = control_buffer(buffer, addr, method, params);

    if (length < 0)
    {
        return length;
    }

    if (sp_socket_write(sock, buffer, length) <= 0)
    {
        return -4;
    }

    return 0;
}

static int control_buffer(byte *buffer, frame_376_address_t *addr, const char *method, sp_json_t *params)
{
    if (sp_string_equal(method, "stop"))
    {
        return frame376_buffer_control_stop(buffer, addr);
    }
    else if (sp_string_equal(method, "power_control"))
    {
        return frame376_buffer_control_power(buffer, addr, params);
    }

    return -2;
}
