#include "collect.h"
#include "protocol_376.h"

typedef struct
{
    char *name;
    int (*func)();
} test_case_t;


static int test_376();
static int test_control();
static int test_376_ex();
static int test_other();
static int test_f5();
static int test_f185();
static int test_f217();
static int test_f218();
static int test_f220();
static int test_f227();
static int test_f97();
static int test_f227();
static int test_rpc();

static test_case_t test_cases[] = {
    //{"test_376_ex", test_376_ex},
    //{"test_376", test_376},
    //{"test_control", test_control},
    //{"test_other", test_other},
    //{"test_f5", test_f5},
    //{"test_f97", test_f97},
    //{"test_f185", test_f185},
    //{"test_f217", test_f217},
    //{"test_f218", test_f218},
    //{"test_f220", test_f220},
    //{"test_f227", test_f227},
      {"test_rpc", test_rpc},
};


int main()
{
    app_init(0, NULL);

    int i;
    for (i = 0; i < sizeof(test_cases) / sizeof(test_case_t); i++)
    {
        test_case_t *ts = &test_cases[i];
        if (ts->func() < 0)
        {
            printf("%s failed\n", ts->name);
        }
        else
        {
            printf("%s success\n", ts->name);
        }
    }

    sp_usleep(1000 * 1000);
    app_fini();
    return 0;
}

static int test_376()
{
    short len;;
    const int length = 1024;
    len = frame376_int_2_length(length);
    if (frame376_length_2_int(len) != length)
    {
        return -1;
    }

    //link test
    //68 3A 00 3A 00 68 E9 01 32 01 00 00 02 71 00 00 01 00 1E 0A B9 16 
    //68 32 00 32 00 68 00 01 32 01 00 02 00 61 00 00 01 00 98 16
    //                  c  a1    a2    a3       da    dt

    //report
    //68 86 00 86 00 68 E4 01 32 01 00 00 0D 60 01 01 09 0B 30 17 28 03 18 01 01 EE EE 30 17 28 03 18 01 01 EE EE EE 1E 0A 81 16 
    //68 32 00 32 00 68 00 01 32 01 00 02 00 68 00 00 01 00 9F 16

    unsigned char buffer[1024] = {0x68, 0x3A, 0x00, 0x3A, 0x00,
        0x68, 0xE9, 0x01, 0x32,
        0x01, 0x00, 0x00, 0x02,
        0x71, 0x00, 0x00, 0x01,
        0x00, 0x1E, 0x0A, 0xB9,
        0x16};
    int size = 22;

    protocol_parser_t *parser = protocol_parser(PROTOCOL_TYPE_376);

    session_t session = {0, };

    parser->handle_input(buffer, size, &session);
    //parser->handle_input(buffer2, size2, &session);

    //F5~F8                      10                            20                            30                            40                            50                            60
    //68 f2 00 f2 00 68 E4 01 32 01 00 00 0D 60 01 01 F0 00 08 04 18 01 00 00 05 00 00 00 05 00 08 04 18 01 00 00 05 00 00 00 05 00 08 04 18 01 00 00 05 00 00 00 05 00 08 04 18 01 00 00 05 00 00 00 05 00 81 16 
    //68 32 00 32 00 68 00 01 32 01 00 02 00 68 00 00 01 00 9F 16
    //                                                dt
    
    sp_usleep(1000 * 1000);

    return 0;
}

static int test_control()
{
    unsigned char buffer[32];
    frame_376_address_t addr;
    addr.a1 = 3201;
    addr.a2 = 1;
    addr.pn = 1;
    int length = frame376_buffer_control_stop(buffer, &addr);
    app_print_payload(buffer, length);
    return 0;
}

static int test_376_ex()
{

    session_t session = {0, };

    sp_usleep(1000 * 1000);

    return 0;
}

static int test_other()
{
    unsigned char b[2] = {1, 0};

    short err = *b;

    printf("%d\n", err);

    sp_json_t *json = sp_json_object_new();
    sp_json_object_add(json, "type", sp_json_string("report"));
    sp_json_object_add(json, "addr", sp_json_string("1"));
    sp_json_object_add(json, "area", sp_json_string("3201"));

    uint64_t enter = sp_timeofday();

    int i;
    for (i = 0; i < 1000; i++)
        app_userdata_report(app_context()->meter_daily_curve, json);

    uint64_t leave = sp_timeofday();
    app_log_warn(

    sp_json_free(json);

    return 0;
}

static int test_f5()
{
    protocol_parser_t *parser = protocol_parser(PROTOCOL_TYPE_376);
    session_t session = {0, };

    unsigned char buffer3[1024] = {
        0x68, 0xf2, 0x00, 0xf2, 0x00,
        0x68, 0xE4, 0x01, 0x32, 0x01,
        0x00, 0x00, 0x0D, 0x60, 0x01,
        0x01, 0xF0, 0x00, 0x08, 0x04,
        0x18, 0x01, 0x00, 0x50, 0x05,
        0x00, 0x00, 0x50, 0x05, 0x00,
        0x08, 0x04, 0x18, 0x01, 0x00, 0x50, 0x05, 0x00, 0x00, 0x50, 0x05, 0x00,
        0x08, 0x04, 0x18, 0x01, 0x00, 0x50, 0x05, 0x00, 0x00, 0x50, 0x05, 0x00,
        0x08, 0x04, 0x18, 0x01, 0x00, 0x50, 0x05, 0x00, 0x00, 0x50, 0x05, 0x00,
        0xff, 0x16};
    int size3 = 68;
    parser->handle_input(buffer3, size3, &session);
    return 0;
}

static int test_f185()
{
    protocol_parser_t *parser = protocol_parser(PROTOCOL_TYPE_376);
    session_t session = {0, };

    unsigned char buffer4[1024] = {
        0x68, 0xa2, 0x01, 0xa2, 0x01,
        0x68, 0xE4, 0x01, 0x32, 0x01,
        0x00, 0x00, 0x0D, 0x60, 0x01,
        0x01, 0x0f, 23, 
        0x08, 0x04, 0x18, 0x30, 0x09, 0x08, 0x04, 0x18, 0x01, 0x00, 0x50, 0x05, 0x05, 0x09, 0x08, 0x04, 0x00, 0x50, 0x05, 0x05, 0x09, 0x08, 0x04,
        0x08, 0x04, 0x18, 0x30, 0x09, 0x08, 0x04, 0x18, 0x01, 0x00, 0x50, 0x05, 0x05, 0x09, 0x08, 0x04, 0x00, 0x50, 0x05, 0x05, 0x09, 0x08, 0x04,
        0x08, 0x04, 0x18, 0x30, 0x09, 0x08, 0x04, 0x18, 0x01, 0x00, 0x50, 0x05, 0x05, 0x09, 0x08, 0x04, 0x00, 0x50, 0x05, 0x05, 0x09, 0x08, 0x04,
        0x08, 0x04, 0x18, 0x30, 0x09, 0x08, 0x04, 0x18, 0x01, 0x00, 0x50, 0x05, 0x05, 0x09, 0x08, 0x04, 0x00, 0x50, 0x05, 0x05, 0x09, 0x08, 0x04,
        0xff, 0x16};
    int size4 = 112;
    parser->handle_input(buffer4, size4, &session);
    return 0;
}
static int test_f217()
{
    
    //f217
    unsigned char buffer2[1024] = {
            0x68, 0x92, 0x00, 0x92, 0x00, 0x68,
            0xE4, 0x01, 0x32, 0x01,
            0x00, 0x00, 0x0D, 0x60,
            0x01, 0x01,
            0x01, 27, 
            0x00, 0x01, 
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01,
            0x00,
            0x00,
            0x01,
            0x01, 0x01, 0x01, 0x01,
            0x30, 0x30, 0x09, 0x10, 0x04, 0x18, 
            0xff, 0x16};
    int size2 = 44;

    protocol_parser_t *parser = protocol_parser(PROTOCOL_TYPE_376);
    session_t session = {0, };
    parser->handle_input(buffer2, size2, &session);
    return 0;
}
static int test_f218()
{
    //f218
    unsigned char buffer[1024] = {
        0x68, 0xde, 0x00, 0xde, 0x00, 0x68,
        0xE4, 0x01, 0x32, 0x01,
        0x00, 0x00, 0x0D, 0x60,
        0x01, 0x01,
        0x2, 27, 
        0x00, 0xaa, 0x01, 0x01, 0x01, 0x00,
        0x01, 0x01, 0x01, 0x01, '1', '0', '0', '0',
        0x02, 0x02, 0x02, 0x02,
        0x10, 0x04, 0x18, 
        0x05, 0x05, 0x05,
        0x00, 
        '0', '0', '0', '0','0', '0', '0', '0','0', '0', '0', '0','0', '0', '0', '0', '1',
        0x00,
        0xff, 0x16};
    int size = 63;

    protocol_parser_t *parser = protocol_parser(PROTOCOL_TYPE_376);
    session_t session = {0, };

    parser->handle_input(buffer, size, &session);
    return 0;
}
static int test_f220()
{
    protocol_parser_t *parser = protocol_parser(PROTOCOL_TYPE_376);
    session_t session = {0, };

    //F220

    unsigned char buffer3[1024] = {
        0x68, 0x0e, 0x01, 0x0e, 0x01, 0x68,
        0xE4, 0x01, 0x32, 0x01,
        0x00, 0x00, 0x0D, 0x60,
        0x01, 0x01,
        0x08, 27, 
        0x00, 
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
        0x02, 0x02, 0x02, 0x02,
        '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1',
        0x80,
        0x01, 0x01, 0x01, 0x01,
        0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00,
        0xff, 0x16};
    int size3 = 75;
    parser->handle_input(buffer3, size3, &session);
    return 0;
}

static int test_f97()
{
    protocol_parser_t *parser = protocol_parser(PROTOCOL_TYPE_376);
    session_t session = {0, };

    unsigned char buffer2[1024] = {
        0x68, 0x86, 0x00, 0x86, 0x00,
        0x68, 0xE4, 0x01, 0x32, 0x01, 
        0x00, 0x00, 0x0D, 0x60, 
        0x01, 0x01, 
        0x01, 12, 
        0x00, 0x00, 
        0x28, 0x03, 0x18, 0x01, 0x01,
        0xEE, 0xEE, 0x30, 0x17, 0x28,
        0x03, 0x18, 0x01, 0x01, 0xEE,
        0xEE, 0xEE, 0x1E, 0x0A, 0x81, 0x16};
    int size2 = 41;

    parser->handle_input(buffer2, size2, &session);
    return 0;
}

static int test_f227()
{
    protocol_parser_t *parser = protocol_parser(PROTOCOL_TYPE_376);
    session_t session = {0, };

    short len = frame376_int_2_length(363);
    app_print_payload(&len, 2);

    unsigned char buffer[1024] = {
        0x68, 0xae, 0x05, 0xae, 0x05,
        0x68, 0xE4, 0x01, 0x32, 0x01, 
        0x00, 0x00, 0x0D, 0x60, 
        0x01, 0x01, 
        0x04, 28, 
        0x00, 0xff, //20
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,//60
        0x50,0x50,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0x01, 0x01,
        0x01, 0x01,//130
        0x01, 0x01,
        0x01, 0x01, 0x01,
        0x01, 0x01, 0x01,
        0x01, 0x01, 0x01,
        0x10, 0x20, 0x30, 0x40,
        0x10, 0x20, 0x30, 0x40,
        0x20, 0x20,
        0x10, 0x20, 0x30, 0x40,//155
        //a
        0x00, 0x01,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x01, 
        0x00,
        0x01, 0x01, 0x02, 0x02, 0x01, 0x01, 0x01, 0x02, 0x03, 0x04,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,//card_number
        0x60, 0x80, 0x00, 0x50, 0x00, 0x10, 0x00, 0x20, 0x80,
        0x00, 
        '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', 
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
        //b
        0x00, 0x01,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x01, 
        0x00,
        0x01, 0x01, 0x02, 0x02, 0x01, 0x01, 0x01, 0x02, 0x03, 0x04,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,//card_number
        0x60, 0x80, 0x00, 0x50, 0x00, 0x10, 0x00, 0x20, 0x80,
        0x00, 
        '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', 
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
        0xff, 0x16};
    int size = 371;

    parser->handle_input(buffer, size, &session);
    return 0;
}

static int test_rpc()
{
    rpc_server_run();  

    sp_usleep(1000 * 1000 * 60);

    return 0;
}
