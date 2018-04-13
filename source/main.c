#include "collect.h"

task_context_t *task_context(int type, int index)
{
    app_context_t *ctx = app_context();
    unsigned int task_key = type << 8 | index;

    task_context_t *task = sp_hashtable_get(ctx->tasks, task_key);

    return task;
}

app_context_t *app_context()
{
    static app_context_t ctx;
    return &ctx;
}

int app_init(int argc, const char *argv[])
{
    app_context_t *ctx = app_context();

    ctx->log = sp_log_new("./log.conf", "app");
    sp_return_val_if_fail(ctx->log, -1);

    app_log_warn("log init success");

    const char *conf = argc > 1 ? argv[1] : "./conf.conf";

    ctx->ini = sp_ini_parse_file(conf);
    sp_return_val_if_fail(ctx->ini, -1);

    sp_string_copy(ctx->meter_daily_curve, sp_ini_get_string(ctx->ini, "collect", "meter_daily_curve"));
    sp_string_copy(ctx->meter_daily_reading, sp_ini_get_string(ctx->ini, "collect", "meter_daily_reading"));
    sp_string_copy(ctx->meter_realtime, sp_ini_get_string(ctx->ini, "collect", "meter_realtime"));

    sp_string_copy(ctx->hub_handshake, sp_ini_get_string(ctx->ini, "collect", "hub_handshake"));
    sp_string_copy(ctx->hub_charge_sh, sp_ini_get_string(ctx->ini, "collect", "hub_charge_sh"));
    sp_string_copy(ctx->hub_charge_halt, sp_ini_get_string(ctx->ini, "collect", "hub_charge_halt"));
    sp_string_copy(ctx->hub_realtime20, sp_ini_get_string(ctx->ini, "collect", "hub_realtime2.0"));

    app_log_warn("conf init success");

    ctx->task_tls = sp_tls_new();

#if 1
    ctx->tasks = task_service_pool_new();
    sp_return_val_if_fail(ctx->tasks, -1);

    int task_num = sp_ini_get_int(ctx->ini, "task", "thread");
    ctx->n_collect_task = task_num;

    app_log_warn("collect task thread = %d", task_num);

    int i;
    for (i = 0; i < task_num; i++)
    {
        task_service_pool_put(ctx->tasks, TASKTYPE_COLLECT, i);
    }

    app_log_warn("tasks init success");
#endif
    sp_return_val_if_fail(collect_service_init() == 0, -1);

    app_log_warn("collect service init success");

    sp_return_val_if_fail(rpc_server_new() == 0, -1);

    app_log_warn("rpc server init success");

    app_log_warn("app init success");

    return 0;
}

int app_run()
{
    debug_enter();

    rpc_server_run();

    collect_service_run();

    debug_leave();
    return 0;
}

void app_fini()
{
    debug_enter();

    app_context_t *ctx = app_context();

    task_service_pool_free(ctx->tasks);
    ctx->tasks = NULL;

    debug_leave();

    sp_log_free(ctx->log);
    ctx->log = NULL;
}

unsigned int direct_hash_func(const void *val)
{
    return (unsigned int)val;
}


int direct_equal_func(const void *key1, const void *key2)
{
    return key1 == key2;
}

#ifndef _TEST_
int main(int argc, const char *argv[])
{

    sp_return_val_if_fail(app_init(argc, argv) == 0, -1);

    sp_return_val_if_fail(app_run() == 0, -1);

    app_fini();

    return 0;
}
#endif
