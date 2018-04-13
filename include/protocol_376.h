#ifndef _PROTOCOL_376_H_
#define _PROTOCOL_376_H_

#include <sp.h>

typedef struct
{
    char fix1;
    short len[2];
    char fix2;

    char control;

    short address_a1;
    short address_a2;
    char address_a3;

    /* variable length */
    char payload[0];

    /* ignore 2 bytes */
} __attribute__((packed)) frame_376_t;

typedef struct
{
    short a1;
    short a2;

    int pn;
} frame_376_address_t;

#define FRAME_376_MIN_SIZE (sizeof(frame_376_t) + 2)

int frame376_length_2_int(short length);
short frame376_int_2_length(int length);

unsigned char frame376_checksum(unsigned char *buffer, int size);

int frame376_buffer_control_stop(unsigned char *buffer, frame_376_address_t *address);
int frame376_buffer_control_power(byte *buffer, frame_376_address_t *address, sp_json_t *params);

char *frame376_fn_2_url(int fn);

void frame376_put_task(frame_376_address_t *address, task_t *task);

void int_2_bcd2(byte *payload, int val);

unsigned int frame376_address_hash(frame_376_address_t *addr);

#endif
