#include "protocol.h"

#include "collect.h"

static int handle_104_input(char *buffer, int size);

protocol_parser_t *protocol_104_parser()
{
    static protocol_parser_t parser = {

        .handle_input = handle_104_input
    };

    return &parser;
}

static int handle_104_input(char *buffer, int size)
{
    return 0;
}
