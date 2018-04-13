#ifndef _COLLECT_PROTOCOL_H_
#define _COLLECT_PROTOCOL_H_

typedef struct
{
    int (*handle_input)(char *buffer, int size, void *arg);
} protocol_parser_t;

enum
{
    PROTOCOL_TYPE_UNKNOWN = 0,
    PROTOCOL_TYPE_376,
    PROTOCOL_TYPE_104,
};

int protocol_try_parse(char *buffer, int size);
protocol_parser_t *protocol_parser(int type);

protocol_parser_t *protocol_376_parser();
protocol_parser_t *protocol_104_parser();



#endif
