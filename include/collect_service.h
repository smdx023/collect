#ifndef _COLLECT_SERVICE_H_
#define _COLLECT_SERVICE_H_

#include <sp.h>

int collect_service_init();

int collect_service_run();

void collect_service_fini();

void handle_connect(int sock, void *arg);
void handle_read(int sock, void *arg);

typedef struct
{
    int sock;
    void *ev;
    int protocol_type;
} session_t;

#endif