#include "protocol.h"

#include "collect.h"

static protocol_parser_t *protocol_unknown_parser();
static int handle_unknown_input(char *buffer, int size);

int protocol_try_parse(char *buffer, int size)
{
    return PROTOCOL_TYPE_376;
}

protocol_parser_t *protocol_parser(int type)
{
    if (type == PROTOCOL_TYPE_376)
    {
        return protocol_376_parser();
    }
    else
    {
        return protocol_unknown_parser();
    }
}

static protocol_parser_t *protocol_unknown_parser()
{
    static protocol_parser_t parser = {

        .handle_input = handle_unknown_input
    };

    return &parser;
}

static int handle_unknown_input(char *buffer, int size)
{
    return 0;
}
