#ifndef _TASK_SERVICE_H_
#define _TASK_SERVICE_H_

#include "collect.h"

typedef struct
{
    void (*cb)(void *arg);
    void *arg;
} task_t;

/* task */


/* task service */
void *task_service_new(int type, int instance, sp_thread_callback cb);
void task_service_put_task(void *service, task_t *task);
void task_service_free(void *service);
void *task_service_cb(void *arg);

/* task service pool */
void *task_service_pool_new();
void task_service_pool_free(void *pool);
void task_service_pool_put(void *pool, int type, int instance);
void *task_service_pool_get(void *pool, int type, int instance);

#endif
