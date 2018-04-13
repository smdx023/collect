#include "collect.h"
#include "task_service.h"

void *task_service_new(int type, int instance, sp_thread_callback cb)
{
    task_context_t *ctx = sp_calloc(1, sizeof(task_context_t));

    ctx->type = type;
    ctx->instance = instance;
    ctx->lock = sp_lock_new();
    ctx->queue = sp_queue_new(NULL);
    ctx->thread = sp_thread_new(cb, ctx);

    sp_return_val_if_fail(ctx->lock && ctx->queue && ctx->thread, NULL);

    ctx->addr2sock = sp_hashtable_new(1024,
        direct_hash_func, 
        sp_hashtable_direct_equal,
        NULL, NULL);

    return ctx;
}

void task_service_put_task(void *service, task_t *task)
{
    task_context_t *ctx = (task_context_t *)service;

    sp_lock_acquire(ctx->lock);

    sp_list_push_back(ctx->queue, task);

    sp_lock_release(ctx->lock);
}

void task_service_free(void *service)
{
    task_context_t *ctx = (task_context_t *)service;

    ctx->loop_exit = 1;
    sp_lock_free(ctx->lock);
    sp_queue_free(ctx->queue);
    sp_thread_free(ctx->thread);

    sp_free(service);
}

void *task_service_cb(void *arg)
{
    task_context_t *ctx = (task_context_t *)arg;

    sp_tls_set(app_context()->task_tls, ctx);

    task_context_t *val = app_task_context();

    app_log_warn("task running: (%x, %x)", ctx, val);

    while (!ctx->loop_exit)
    {
        task_t *task = NULL;
        sp_lock_acquire(ctx->lock);
        sp_queue_pop(ctx->queue, &task);
        sp_lock_release(ctx->lock);

        if (!task)
        {
            continue;
        }

        task->cb(task->arg);
        sp_free(task);
    }
}

void *task_service_pool_new()
{
    void *table = sp_hashtable_new(16, direct_hash_func, direct_equal_func, NULL, task_service_free);

    return table;
}

void task_service_pool_free(void *pool)
{
    sp_return_if_fail(pool);

    sp_hashtable_free(pool);
}

void task_service_pool_put(void *pool, int type, int instance)
{
    unsigned int key = type << 8 | instance;

    void *service = task_service_new(type, instance, task_service_cb);
    sp_hashtable_put(pool, key, service);
}

void *task_service_pool_get(void *pool, int type, int instance)
{
    unsigned int key = type << 8 | instance;
    return sp_hashtable_get(pool, key);
}
