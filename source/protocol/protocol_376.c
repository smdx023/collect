#include "protocol.h"

#include "collect.h"
#include "protocol_376.h"

typedef struct
{
    int sock;

    //for report url
    char url[256];

    unsigned char prm;
    unsigned char acd;
    unsigned char fn;

    short a1;
    short a2;
    unsigned char a3;

    unsigned char afn;

    unsigned char tpv;
    unsigned char con;
    unsigned char seq;

    unsigned char da1;
    unsigned char da2;

    unsigned char dt1;
    unsigned char dt2;

    int payload_length;
    unsigned char payload[0];
    
} frame376_object_t;

static int handle_376_input(char *buffer, int size, void *arg);
static int verify_frame(char *buffer, int size);
static void frame_376_reply(frame376_object_t *obj);
static sp_json_t *frame_376_fn(int pn, unsigned char *buffer);
static sp_json_t *frame_376_pn(int fn, unsigned char *buffer);

static void tdc_2_string(char *string, unsigned char *tdc);
static void  tdd_2_string(char *string, unsigned char *tdd);
static void a17_2_string(char *string, unsigned char year, unsigned char *a17);
static double a7_2_float(unsigned char *a7);
static double a25_2_float(unsigned char *a25);
static double a13_2_float(unsigned char *a13);
static double a11_2_float(unsigned char *a11);
static double a23_2_float(unsigned char *a23);
static double a6_2_float(unsigned char *a6);
static double char4_2_float6(unsigned char *payload);
static double char2_2_float1(unsigned char *payload);
static double char2_2_float2(unsigned char *payload);
static double char4_2_float2(unsigned char *payload);
static double char4_2_float3(unsigned char *payload);
static double char4_2_float5(unsigned char *payload);
static sp_json_t *frame_2_json(frame376_object_t *object);
static int fn_2_json(sp_json_t **json, int fn, unsigned char *payload);
static int pn_2_json(sp_json_t **json, frame376_object_t *frame, int fn, unsigned char *payload);
static short address_a1_2_int(short a1);
static short int_2_address_a1(short a1);
static void int_2_string(char *string, int val);
static void add_fn(unsigned char *dt1, unsigned char *dt2, int fn);
static void add_pn(unsigned char *da1, unsigned char *da2, int pn);
static int dt_2_fn(unsigned char dt1, unsigned char dt2);
static unsigned char bcd_2_int(unsigned char bcd);
static short bcd2_2_int(unsigned char bcd[2]);
static void bcd8_2_string(char *string, unsigned char bcd[8]);
static void bcd6_2_string(char *string, unsigned char bcd[6]);
static void bcd16_2_string(char *string, unsigned char bcd[16]);
static void int_2_bcd4(byte *payload, int val);

static sp_json_t *runtime_data_2_json(frame376_object_t *);
static sp_json_t *history_data_2_json(frame376_object_t *);
static sp_json_t *event_data_2_json(frame376_object_t *);

static int fn5_2_json(sp_json_t *json, unsigned char *payload);
static int fn89_2_json(sp_json_t *json, unsigned char *payload);
static int fn92_2_json(sp_json_t *json, unsigned char *payload);
static int fn97_2_json(sp_json_t *json, unsigned char *payload);
static int fn101_2_json(sp_json_t *json, unsigned char *payload);
static int fn185_2_json(sp_json_t *json, unsigned char *payload);

//充电桩静态数据
static int fn212_2_json(sp_json_t *json, unsigned char *payload);
//远程充值，桩请求，主站应答
static int fn213_2_json(sp_json_t *json, unsigned char *payload);
//桩查询未结算记录，桩请求，主站应答
static int fn214_2_json(sp_json_t *json, unsigned char *payload);
//已结算记录，桩请求，主站应答
static int fn215_2_json(sp_json_t *json, unsigned char *payload);
//充电过程报文
static int fn216_2_json(sp_json_t *json, unsigned char *payload);
//充电起停事件
static int fn217_2_json(sp_json_t *json, unsigned char *payload);
//握手事件
static int fn218_2_json(sp_json_t *json, unsigned char *payload);
//充电配置参数
static int fn219_2_json(sp_json_t *json, unsigned char *payload);
//中止充电事件
static int fn220_2_json(sp_json_t *json, unsigned char *payload);
//充电鉴权数据，桩请求，主站应答
static int fn221_2_json(sp_json_t *json, unsigned char *payload);
//充电实时数据V1.0
static int fn222_2_json(sp_json_t *json, unsigned char *payload);
//充电实时数据V1.5
static int fn223_2_json(sp_json_t *json, unsigned char *payload);
//充电交易记录，桩上报，主站应答
static int fn224_2_json(sp_json_t *json, unsigned char *payload);
//充电实时数据V2.0
static int fn227_2_json(sp_json_t *json, unsigned char *payload);
static void task_cb(frame376_object_t *obj);

protocol_parser_t *protocol_376_parser()
{
    static protocol_parser_t parser = {

        .handle_input = handle_376_input
    };

    return &parser;
}

static int handle_376_input(char *buffer, int size, void *arg)
{
    int length = verify_frame(buffer, size);
    sp_return_val_if_fail(length >= 0, -1);

    frame_376_t *frame = (frame_376_t *)buffer;

    unsigned char prm = frame->control >> 6 & 1;
    unsigned char acd = frame->control >> 5 & 1;
    unsigned char fn = frame->control & 0xf;

    short a1 = frame->address_a1;
    short a2 = frame->address_a2;
    unsigned char a3 = frame->address_a3;
    unsigned char msa = frame->address_a3 >> 1;

    unsigned char afn = frame->payload[0];

    //confrim
    unsigned char tpv = frame->payload[1] >> 7;
    unsigned char con = frame->payload[1] >> 4 & 1;
    unsigned char seq = frame->payload[1] & 0xf;

    //pn
    unsigned char da1 = frame->payload[2];
    unsigned char da2 = frame->payload[3];
    //int pn = da2 << 3 + da1;

    unsigned char dt1 = frame->payload[4];
    unsigned char dt2 = frame->payload[5];
    //int fn = dt1 << 3 + dt2;

    //link test
    //68 3A 00 3A 00 68 E9 01 32 01 00 00 02 71 00 00 01 00 1E 0A B9 16 
    //68 32 00 32 00 68 00 01 32 01 00 02 00 61 00 00 01 00 98 16
    //                  c  a1    a2    a3 fn    da    dt

    //report
    //68 86 00 86 00 68 E4 01 32 01 00 00 0D 60 01 01 09 0B 30 17 28 03 18 01 01 EE EE 30 17 28 03 18 01 01 EE EE EE 1E 0A 81 16 
    //68 32 00 32 00 68 00 01 32 01 00 02 00 68 00 00 01 00 9F 16

    printf("prm:%02x, \
        acd:%02x, \
        fn:%02x, \
        msa:%02x, \
        afn:%02x, \
        tpv:%02x, \
        seq:%02x, \
        da1:%02x, \
        con:%02x, \
        da2:%02x, \
        dt1:%02x, \
        dt2:%02x\n",
        prm, 
        acd,
        fn, 
        msa, 
        afn,
        tpv,
        con,
        seq,
        da1,
        da2,
        dt1,
        dt2);

    /* afn + seq + da + dt = 6 bytes */
    int payload_length = length - 6;
    frame376_object_t *obj = sp_calloc(1, sizeof(frame376_object_t) + payload_length);
    obj->a1 = a1;
    obj->a2 = a2;
    obj->a3 = a3;
    obj->prm = prm;
    obj->acd = acd;
    obj->afn = afn;
    obj->fn = fn;
    obj->con = con;
    obj->da1 = da1;
    obj->da2 = da2;
    obj->dt1 = dt1;
    obj->dt2 = dt2;
    obj->seq = seq;
    obj->tpv = tpv;

    session_t *session = (session_t *)arg;
    obj->sock = session->sock;

    obj->payload_length = payload_length;
    sp_copy(obj->payload, frame->payload + 6, payload_length);


    task_t *task = sp_malloc(sizeof(task_t));
    task->arg = obj;
    task->cb = task_cb;

    frame_376_address_t address;
    address.a1 = obj->a1;
    address.a2 = obj->a2;
    frame376_put_task(&address, task);
    
    return 0;
}

void frame376_put_task(frame_376_address_t *address, task_t *task)
{
    app_context_t *ctx = app_context();
    unsigned int instance = ((address->a1 << 16) | address->a2) % ctx->n_collect_task;

    void *task_service = task_service_pool_get(ctx->tasks, TASKTYPE_COLLECT, instance);

    task_service_put_task(task_service, task);
}

int frame376_length_2_int(short length)
{
    return length >> 2;
}

short frame376_int_2_length(int length)
{
    return length << 2 | 2;
}

unsigned char frame376_checksum(unsigned char *buffer, int size)
{
    int i;
    unsigned char checksum = 0;

    for (i = 0; i < size; i++)
    {
        checksum += buffer[i];
    }

    return checksum;
}

static int verify_frame(char *buffer, int size)
{
    if (size < FRAME_376_MIN_SIZE)
    {
        return -1;
    }
    if (buffer[size - 1] != 0x16)
    {
        return -1;
    }

    frame_376_t *frame = (frame_376_t *)buffer;
    if (frame->fix1 != 0x68 || frame->fix2 != 0x68)
    {
        return -1;
    }

    if (frame->len[0] != frame->len[1])
    {
        return -1;
    }

    int length = frame376_length_2_int(frame->len[0]);

    /* length == size - tail(2bytes) - head(6bytes) */
    if (length != size - 8)
    {
        return -1;
    }

    /* fixme: verify cs */

    /* payload length, length - control(1bytes) - address(5bytes) */
    return length - 6;
}

static void task_cb(frame376_object_t *frame)
{
    //link test
    //68 3A 00 3A 00 68 E9 01 32 01 00 00 02 71 00 00 01 00 1E 0A B9 16 
    //68 32 00 32 00 68 00 01 32 01 00 02 00 61 00 00 01 00 98 16
    //                  c  a1    a2    a3 fn    da    dt

    //report
    //68 86 00 86 00 68 E4 01 32 01 00 00 0D 60 01 01 09 0B 30 17 28 03 18 01 01 EE EE 30 17 28 03 18 01 01 EE EE EE 1E 0A 81 16 
    //68 32 00 32 00 68 00 01 32 01 00 02 00 68 00 00 01 00 9F 16

    /*
    * FN
    * PRM = 1
    * 0 BAK
    * 1 RESET
    * 4 USER DATA   REQ
    * 9 LINK TEST   REQ/RES
    * 10 ASK LEVEL-1 DATA REQ/RES
    * 11 ASK LEVEL-2 DATA REQ/RES
    */

    /*
    * AFN
    * 00H CONFIRM
    * 01H RESET
    * 02H LINK TEST
    * 03H RELAY
    * 04H SET PARAMS
    * 05H CONTROL
    * 06H AUTH
    * 07H BAK
    * 08H ASK TERMINAL REPORT
    * 09H ASK TERMINAL SETTINGS
    * 0AH ASK PARAMS
    * 0BH ASK TASK DATA
    * 0CH ASK LEVEL-1 DATA RUNTIME DATA
    * 0DH ASK LEVEL-2 DATA HISTORY DATA
    * 0EH ASK LEVEL-3 DATA EVENT DATA
    * 0FH FILE TRANS
    * 10H DATA TRANSFER
    * OTHER: BAK
    */
    if (frame->afn == 0x00)
    {
        /* reply from terminal
        * set status/orher op if necessary
        */
    }
    else if (frame->afn == 0x02)
    {
        int fn = dt_2_fn(frame->dt1, frame->dt2);
        if (fn == 1)
        {
            //login
            //report status online
        }
        else if (fn == 2)
        {
            //logout
            //report status offline ?
        }
        else if (fn == 3)
        {
            //heartbeat
        }
    }
    else if (frame->afn == 0x0c)
    {
        sp_json_t *json = frame_2_json(frame);
        app_userdata_report(frame->url, json);
        sp_json_free(json);
    }
    else if (frame->afn == 0x0d)
    {
        sp_json_t *json = frame_2_json(frame);
        
        app_userdata_report(frame->url, json);
        sp_json_free(json);
    }
    else if (frame->afn == 0x0e)
    {
        sp_json_t *json = frame_2_json(frame);
        app_userdata_report(frame->url, json);
        sp_json_free(json);
    }
    else
    {
        printf("invalid payload?\n");
    }

    if (frame->con)
    {
        frame_376_reply(frame);
    }

    task_context_t *ctx = app_task_context();
    frame_376_address_t addr;
    addr.a1 = frame->a1;
    addr.a2 = frame->a2;
    unsigned int key = frame376_address_hash(&addr);
    sp_hashtable_put(ctx->addr2sock, key, frame->sock);

    sp_free(frame);
}

static void frame_376_reply(frame376_object_t *obj)
{
    //link test
    //68 3A 00 3A 00 68 E9 01 32 01 00 00 02 71 00 00 01 00 1E 0A B9 16 
    //68 32 00 32 00 68 00 01 32 01 00 02 00 61 00 00 01 00 98 16
    //                  c  a1    a2    a3 fn    da    dt

    //report
    //68 86 00 86 00 68 E4 01 32 01 00 00 0D 60 01 01 09 0B 30 17 28 03 18 01 01 EE EE 30 17 28 03 18 01 01 EE EE EE 1E 0A 81 16 
    //68 32 00 32 00 68 00 01 32 01 00 02 00 68 00 00 01 00 9F 16
//    if (obj->afn == 0x02)
    {
        unsigned char buffer[20];
        frame_376_t *frame = (frame_376_t *)buffer;
        frame->fix1 = 0x68;
        frame->fix2 = 0x68;
        frame->len[0] = frame376_int_2_length(12);
        frame->len[1] = frame->len[0];
        frame->control = 0;
        frame->address_a1 = obj->a1;
        frame->address_a2 = obj->a2;
        frame->address_a3 = 2;
        frame->payload[0] = 0;/*AFN: 0 CONFIRM*/
        frame->payload[1] = (6 << 4) | obj->seq;/* SEQ */
        /* da1 = da2 = 0, CONFIRM ALL pn */
        frame->payload[2] = 0;
        frame->payload[3] = 0;
        /* F1 confirm all, F2, refuse all */
        frame->payload[4] = 1;
        frame->payload[5] = 0;
        frame->payload[6] = frame376_checksum(&frame->control, 12);
        frame->payload[7] = 0x16;

        app_print_payload(buffer, 20);
        sp_socket_write(obj->sock, frame, 20);
    }
}

static sp_json_t *frame_376_fn(int pn, unsigned char *buffer)
{
    sp_return_val_if_fail(buffer, NULL);
}

static sp_json_t *frame_376_pn(int fn, unsigned char *buffer)
{
    sp_return_val_if_fail(buffer, NULL);
}

static void tdc_2_string(char *string, unsigned char *tdc)
{
    sp_string_clear(string);
    sp_string_append(string, "%02d%02d%02d%02d%02d",
        ((tdc[4] >> 4) * 10) + (tdc[4] & 0xf),
        ((tdc[3] >> 4) * 10) + (tdc[3] & 0xf),
        ((tdc[2] >> 4) * 10) + (tdc[2] & 0xf),
        ((tdc[1] >> 4) * 10) + (tdc[1] & 0xf),
        ((tdc[0] >> 4) * 10) + (tdc[0] & 0xf));
}

static void  tdd_2_string(char *string, unsigned char *tdd)
{
    sp_string_clear(string);

    sp_string_append(string, "%02d%02d%02d0000",
        ((tdd[2] >> 4) * 10) + (tdd[2] & 0xf),
        ((tdd[1] >> 4) * 10) + (tdd[1] & 0xf),
        ((tdd[0] >> 4) * 10) + (tdd[0] & 0xf));
}

static void a17_2_string(char *string, unsigned char year, unsigned char *a17)
{
    sp_string_clear(string);

    sp_string_append(string, "%02d%02d%02d%02d%02d",
        ((year >> 4) * 10) + (year & 0xf),
        ((a17[3] >> 4) * 10) + (a17[3] & 0xf),
        ((a17[2] >> 4) * 10) + (a17[2] & 0xf),
        ((a17[1] >> 4) * 10) + (a17[1] & 0xf),
        ((a17[0] >> 4) * 10) + (a17[0] & 0xf));
}

static double a7_2_float(unsigned char *a7)
{
    if (a7[0] == 0xEE || a7[1] == 0xEE)
    {
        return 0.0f;
    }

    return (a7[1] >> 4) * 100
        + (a7[1] & 0xf) * 10
        + (a7[0] >> 4)
        + (a7[0] & 0xf) / 10.0;
}

static double a25_2_float(unsigned char *a25)
{
    if (a25[0] == 0xEE || a25[1] == 0xEE || a25[2] == 0xEE)
    {
        return 0.0f;
    }

    return (a25[0] & 0xf) / 1000.0
        + (a25[0] >> 4) / 100.0
        + (a25[1] & 0xf) / 10.0
        + (a25[1] >> 4)
        + (a25[2] & 0xf) * 10
        + (a25[2] >> 4) * 100;
}

static sp_json_t *frame_2_json(frame376_object_t *frame)
{
    sp_json_t *json = sp_json_object_new();

    char string[16];
    int_2_string(string, address_a1_2_int(frame->a1));
    sp_json_object_add(json, "area", sp_json_string(string));

    int_2_string(string, frame->a2);
    sp_json_object_add(json, "addr", sp_json_string(string));

    sp_json_object_add(json, "type", sp_json_string("report"));
    /* FN = 4, AFN = 0D, history report */
    if (frame->afn == 0x0d)
    {
        sp_json_t *pn_data = history_data_2_json(frame);
        sp_json_object_add(json, "data", pn_data);
    }
    else if (frame->afn == 0x0c)
    {
        sp_json_t *pn_data = runtime_data_2_json(frame);
        sp_json_object_add(json, "data", pn_data);
    }
    else if (frame->afn == 0x0e)
    {
        sp_json_t *pn_data = event_data_2_json(frame);
        sp_json_object_add(json, "data", pn_data);
    }

    return json;
}

static int fn_2_json(sp_json_t **json, int fn, unsigned char *payload)
{
    sp_json_t *fn_json = sp_json_object_new();

    char string[16];
    int_2_string(string, fn);
    sp_json_object_add(fn_json, "fn", sp_json_string(string));
    int length = 0;

    switch (fn)
    {
        case 89:
            length = fn89_2_json(fn_json, payload);
            break;
        case 92:
            length = fn92_2_json(fn_json, payload);
            break;
        case 97:
        case 98:
        case 99:
        case 100:
            length = fn97_2_json(fn_json, payload);
            break;
        case 101:
        case 102:
        case 103:
        case 104:
            length = fn101_2_json(fn_json, payload);
            break;
        case 5:
        case 6:
        case 7:
        case 8:
            length = fn5_2_json(fn_json, payload);
            break;
        case 185:
        case 186:
        case 187:
        case 188:
            length = fn185_2_json(fn_json, payload);
            break;
        case 212:
            length = fn212_2_json(fn_json, payload);
            break;
        case 213:
            length = fn213_2_json(fn_json, payload);
            break;
        case 214:
            length = fn214_2_json(fn_json, payload);
            break;
        case 215:
            length = fn215_2_json(fn_json, payload);
            break;
        case 216:
            length = fn216_2_json(fn_json, payload);
            break;
        case 217:
            length = fn217_2_json(fn_json, payload);
            break;
        case 218:
            length = fn218_2_json(fn_json, payload);
            break;
        case 219:
            length = fn219_2_json(fn_json, payload);
            break;
        case 220:
            length = fn220_2_json(fn_json, payload);
            break;
        case 221:
            length = fn221_2_json(fn_json, payload);
            break;
        case 222:
            length = fn222_2_json(fn_json, payload);
            break;
        case 223:
            length = fn223_2_json(fn_json, payload);
            break;
        case 224:
            length = fn224_2_json(fn_json, payload);
            break;
        case 227:
            length = fn227_2_json(fn_json, payload);
            break;
        default:
            app_log_error("unexpected fn:%d", fn);
            break;
    }

    *json = fn_json;

    return length;
}

static int pn_2_json(sp_json_t **json, frame376_object_t *frame, int pn, unsigned char *payload)
{
    sp_json_t *pn_json = sp_json_object_new();

    char string[16];
    int_2_string(string, pn);
    sp_json_object_add(pn_json, "pn", sp_json_string(string));

    sp_json_t *fn_array = sp_json_array_new();
    sp_json_object_add(pn_json, "data", fn_array);

    unsigned char fn_pos = 1;
    int j;
    for (j = 0; j < 8; j++)
    {
        if (frame->dt1 & fn_pos)
        {
            int fn = (frame->dt2 << 3) + j + 1;

            sp_string_copy(frame->url, frame376_fn_2_url(fn));

            sp_json_t *fn_json = NULL;
            payload += fn_2_json(&fn_json, fn, payload);
            sp_json_array_add(fn_array, fn_json);
        }

        fn_pos = fn_pos << 1;
    }

    *json = pn_json;
}

static double a13_2_float(unsigned char *a13)
{
    double val = (a13[0] & 0xf) / 10000.0
        + (a13[0] >> 4) / 1000.0
        + (a13[1] & 0xf) / 100.0
        + (a13[1] >> 4) / 10.0
        + (a13[2] & 0xf)
        + (a13[2] >> 4) * 10
        + (a13[3] & 0xf) * 100
        + (a13[3] >> 4) * 1000;

    return val;
}

static double a11_2_float(unsigned char *a11)
{
    return (a11[0] & 0xf) / 100.0
        + (a11[0] >> 4) / 10.0
        + (a11[1] & 0xf)
        + (a11[1] >> 4) * 10
        + (a11[2] & 0xf) * 100
        + (a11[2] >> 4) * 1000
        + (a11[3] & 0xf) * 10000
        + (a11[3] >> 4) * 100000;
}

static double a23_2_float(unsigned char *a23)
{
    return (a23[0] & 0xf) / 10000.0
        + (a23[0] >> 4) / 1000.0
        + (a23[1] & 0xf) / 100.0
        + (a23[1] >> 4) / 10.0
        + (a23[2] & 0xf)
        + (a23[2] >> 4) * 10;
}

static double a6_2_float(unsigned char *a6)
{
    return char2_2_float2(a6);
}

static double char4_2_float6(unsigned char *payload)
{
    return (payload[0] & 0xf) / 1000000.0
        + (payload[0] >> 4) / 100000.0
        + (payload[1] & 0xf) / 10000.0
        + (payload[1] >> 4) / 1000.0
        + (payload[2] & 0xf) / 100.0
        + (payload[2] >> 4) / 10.0
        + (payload[3] & 0xf)
        + (payload[3] >> 4) * 10;
}

static double char2_2_float1(unsigned char *payload)
{
    return (payload[0] & 0xf) / 10.0
        + (payload[0] >> 4)
        + (payload[1] & 0xf) * 10
        + (payload[1] >> 4) * 100;
}

static double char2_2_float2(unsigned char *payload)
{
    return (payload[0] & 0xf) / 100.0
        + (payload[0] >> 4) / 10.0
        + (payload[1] & 0xf)
        + (payload[1] >> 4) * 100;
}

static double char4_2_float2(unsigned char *payload)
{
    return a11_2_float(payload);
}

static double char4_2_float3(unsigned char *payload)
{
    return (payload[0] & 0xf) / 1000.0
        + (payload[0] >> 4) / 100.0
        + (payload[1] & 0xf) / 10.0
        + (payload[1] >> 4)
        + (payload[2] & 0xf) * 10
        + (payload[2] >> 4) * 100;
}

static double char4_2_float5(unsigned char *payload)
{
    return a13_2_float(payload);
}

static short address_a1_2_int(short address_a1)
{
    short a1 = (address_a1 & 0xf)
        + (address_a1 >> 4 & 0xf) * 10
        + (address_a1 >> 8 & 0xf) * 100
        + (address_a1 >> 12) * 1000;

    return a1;
}

static short bcd2_2_int(unsigned char bcd[2])
{
    short val = (bcd[0] & 0xf)
        + (bcd[0] >> 4 & 0xf) * 10
        + (bcd[1] >> 8 & 0xf) * 100
        + (bcd[1] >> 12) * 1000;

    return val;
}

static unsigned char bcd_2_int(unsigned char bcd)
{
    unsigned char val = (bcd & 0xf)
        + (bcd >> 4) * 10;

    return val;
}

static void bcd8_2_string(char *string, unsigned char bcd[8])
{
    sp_string_clear(string);

    int i;
    for (i = 7; i >= 0; i--)
    {
        unsigned char bcd0 = bcd[i] & 0xf;
        unsigned char bcd1 = (bcd[i] >> 4 & 0xf);

        if (bcd0 == 0xf)
        {
            continue;
        }

        if (bcd1 == 0xf)
        {
            sp_string_append(string, "%d", bcd0);
            continue;
        }

        sp_string_append(string, "%d", bcd1 * 10 + bcd0);
    }
}

static void bcd6_2_string(char *string, unsigned char bcd[6])
{
    sp_string_clear(string);

    sp_string_append(string, "%02d%02d%02d%02d%02d%02d",
        ((bcd[5] >> 4) * 10) + (bcd[5] & 0xf),
        ((bcd[4] >> 4) * 10) + (bcd[4] & 0xf),
        ((bcd[3] >> 4) * 10) + (bcd[3] & 0xf),
        ((bcd[2] >> 4) * 10) + (bcd[2] & 0xf),
        ((bcd[1] >> 4) * 10) + (bcd[1] & 0xf),
        ((bcd[0] >> 4) * 10) + (bcd[0] & 0xf));
}

static void bcd16_2_string(char *string, unsigned char bcd[16])
{
    sp_string_clear(string);
    sp_string_append(string, "%02d%02d%02d%02d%02d%02d%02d%02d%02d%02d%02d%02d%02d%02d%02d%02d",
        ((bcd[15] >> 4) * 10) + (bcd[15] & 0xf),
        ((bcd[14] >> 4) * 10) + (bcd[14] & 0xf),
        ((bcd[13] >> 4) * 10) + (bcd[13] & 0xf),
        ((bcd[12] >> 4) * 10) + (bcd[12] & 0xf),
        ((bcd[11] >> 4) * 10) + (bcd[11] & 0xf),
        ((bcd[10] >> 4) * 10) + (bcd[10] & 0xf),
        ((bcd[9] >> 4) * 10) + (bcd[9] & 0xf),
        ((bcd[8] >> 4) * 10) + (bcd[8] & 0xf),
        ((bcd[7] >> 4) * 10) + (bcd[7] & 0xf),
        ((bcd[6] >> 4) * 10) + (bcd[6] & 0xf),
        ((bcd[5] >> 4) * 10) + (bcd[5] & 0xf),
        ((bcd[4] >> 4) * 10) + (bcd[4] & 0xf),
        ((bcd[3] >> 4) * 10) + (bcd[3] & 0xf),
        ((bcd[2] >> 4) * 10) + (bcd[2] & 0xf),
        ((bcd[1] >> 4) * 10) + (bcd[1] & 0xf),
        ((bcd[0] >> 4) * 10) + (bcd[0] & 0xf)
        );
}

static void int_2_bcd4(byte *payload, int val)
{
    int v = val % 100;
    payload[0] = v % 10 | ((v / 10) << 4);

    v = val / 100 % 100;
    payload[1] = v % 10 | ((v / 10) << 4);

    v = val / 10000 % 100;
    payload[2] = v % 10 | ((v / 10) << 4);

    v = val / 1000000 % 100;
    payload[3] = v % 10 | ((v / 10) << 4);
}

void int_2_bcd2(byte *payload, int val)
{
    int v = val % 100;
    payload[0] = v % 10 | ((v / 10) << 4);

    v = val / 100;
    payload[1] = v % 10 | ((v / 10) << 4);
}

static short int_2_address_a1(short a1)
{
    short address_a1 = (a1 / 1000 << 12)
        + (a1 % 1000 / 100 << 8)
        + (a1 % 100 / 10 << 4)
        + (a1 % 10);

    return address_a1;
}

static void int_2_string(char *string, int val)
{
    sp_string_clear(string);
    sp_string_append(string, "%d", val);
}

static void add_fn(unsigned char *dt1, unsigned char *dt2, int fn)
{
    *dt2 = (fn >> 3);
    int shift = fn % 8;
    if (shift > 0)
    {
        *dt1 |= (1 << (shift - 1));
    }
}

static void add_pn(unsigned char *da1, unsigned char *da2, int pn)
{
    if (pn == 0)
    {
        *da1 = 0;
        *da2 = 0;
        return;
    }

    *da2 = (pn >> 3) + 1;
    int shift = pn % 8;
    if (shift > 0)
    {
        *da1 |= (1 << (shift - 1));
    }
}

static int dt_2_fn(unsigned char dt1, unsigned char dt2)
{
    int i;
    unsigned char pos = 1;

    for (i = 0; i < 8; i++)
    {
        if (dt1 & pos)
        {
            int fn = (dt2 << 3) + i + 1;
            return fn;
        }

        pos << 1;
    }

    return (dt2 << 3);
}

int frame376_buffer_control_stop(unsigned char *buffer, frame_376_address_t *address)
{
    frame_376_t *frame = (frame_376_t *)buffer;
    frame->fix1 = 0x68;
    frame->fix2 = 0x68;
    frame->len[0] = frame376_int_2_length(20);
    frame->len[1] = frame->len[0];
    frame->address_a1 = int_2_address_a1(address->a1);
    frame->address_a2 = address->a2;
    frame->address_a3 = 2;
    frame->control = (4 << 4) | 0x0b;
    frame->payload[0] = 0x05;/* AFN: control */
    frame->payload[1] = (7 << 4) | 1;

    frame->payload[2] = 0;
    frame->payload[3] = 0;
    add_pn(&frame->payload[2], &frame->payload[3], address->pn);

    frame->payload[4] = 0;
    frame->payload[5] = 0;
    add_fn(&frame->payload[4], &frame->payload[5], 49);

    frame->payload[6] = 1;
    sp_bzero(&frame->payload[7], 7);

    frame->payload[14] = frame376_checksum(&frame->control, 20);
    frame->payload[15] = 0x16;

    return 28;
}

int frame376_buffer_control_power(byte *buffer, frame_376_address_t *address, sp_json_t *params)
{
    frame_376_t *frame = (frame_376_t *)buffer;
    frame->fix1 = 0x68;
    frame->fix2 = 0x68;
    frame->len[0] = frame376_int_2_length(32);
    frame->len[1] = frame->len[0];
    frame->address_a1 = int_2_address_a1(address->a1);
    frame->address_a2 = address->a2;
    frame->address_a3 = 2;
    frame->control = (4 << 4) | 0x0b;
    frame->payload[0] = 0x05;/* AFN: control */
    frame->payload[1] = (7 << 4) | 1;

    frame->payload[2] = 0;
    frame->payload[3] = 0;
    add_pn(&frame->payload[2], &frame->payload[3], address->pn);

    frame->payload[4] = 0;
    frame->payload[5] = 0;
    add_fn(&frame->payload[4], &frame->payload[5], 46);

    sp_json_t *item = sp_json_object_item(params, "gun");
    sp_return_val_if_fail(item, -1);

    frame->payload[6] = item->valueint;

    item = sp_json_object_item(params, "disable");
    sp_return_val_if_fail(item, -1);

    frame->payload[7] = item->valueint;

    item = sp_json_object_item(params, "group_power_max");
    sp_return_val_if_fail(item, -1);

    int_2_bcd4(&frame->payload[8], item->valueint);

    item = sp_json_object_item(params, "group_hub_count");
    frame->payload[12] = (item->valueint & 0xff);
    frame->payload[13] = (item->valueint >> 8);

    item = sp_json_object_item(params, "group_power_output");
    sp_return_val_if_fail(item, -1);

    int_2_bcd4(&frame->payload[14], item->valueint);

    item = sp_json_object_item(params, "group_power_left");
    sp_return_val_if_fail(item, -1);

    int_2_bcd4(&frame->payload[18], item->valueint);

    item = sp_json_object_item(params, "hub_power_max");
    sp_return_val_if_fail(item, -1);

    int_2_bcd4(&frame->payload[22], item->valueint);

    frame->payload[26] = frame376_checksum(&frame->control, 32);
    frame->payload[27] = 0x16;

    return 40;    
}

char *frame376_fn_2_url(int fn)
{
    app_context_t *ctx = app_context();
    char *url = ctx->meter_realtime;

    switch (fn)
    {
        case 5:
        case 6:
        case 7:
        case 8:
            url = ctx->meter_daily_reading;
            break;
        case 97:
        case 98:
        case 99:
        case 100:
        case 101:
        case 102:
        case 103:
        case 104:
            url = ctx->meter_realtime;
            break;
        case 185:
        case 186:
        case 187:
        case 188:
            url = ctx->meter_daily_curve;
            break;
        case 217:
            url = ctx->hub_charge_sh;
            break;
        case 218:
            url = ctx->hub_handshake;
            break;
        case 220:
            url = ctx->hub_charge_halt;
            break;
        case 227:
            url = ctx->hub_realtime20;
            break;
        default:
            break;
    }

    app_log_debug("fn(%d) -> url(%s)", fn, url);

    return url;
}

static sp_json_t *runtime_data_2_json(frame376_object_t *object)
{
    return history_data_2_json(object);
}

static sp_json_t *history_data_2_json(frame376_object_t *object)
{
    sp_json_t *pn_data = sp_json_array_new();
    int i, j;
    unsigned char pn_pos = 1;

    unsigned char *payload = object->payload;
    for (i = 0; i < 8; i++)
    {
        if (object->da1 & pn_pos)
        {
            int pn = ((object->da2 - 1) << 3) + i + 1;
            sp_json_t *pn_json = NULL;
            payload += pn_2_json(&pn_json, object, pn, payload);
            sp_json_array_add(pn_data, pn_json);
        }

        pn_pos = pn_pos << 1;
    }

    return pn_data;
}

static sp_json_t *event_data_2_json(frame376_object_t *object)
{
    return history_data_2_json(object);
}

static int fn89_2_json(sp_json_t *json, unsigned char *payload)
{
    char timestamp[16];
    tdc_2_string(timestamp, payload);
    payload += 7;

    double val = a7_2_float(payload);
    payload += 2;

    sp_json_object_add(json, "timestamp", sp_json_string(timestamp));
    sp_json_object_add(json, "val", sp_json_double(val));

    return 9;
}

static int fn92_2_json(sp_json_t *json, unsigned char *payload)
{
    char timestamp[16];
    tdc_2_string(timestamp, payload);
    payload += 7;

    double val = a25_2_float(payload);
    payload += 3;

    sp_json_object_add(json, "timestamp", sp_json_string(timestamp));
    sp_json_object_add(json, "val", sp_json_double(val));

    return 10;    
}

static int fn97_2_json(sp_json_t *json, unsigned char *payload)
{
    char timestamp[16];
    tdc_2_string(timestamp, payload);
    payload += 7;

    double val = a13_2_float(payload);
    payload += 4;

    sp_json_object_add(json, "timestamp", sp_json_string(timestamp));
    sp_json_object_add(json, "val", sp_json_double(val));

    return 11;
}

static int fn101_2_json(sp_json_t *json, unsigned char *payload)
{
    char timestamp[16];
    tdc_2_string(timestamp, payload);
    payload += 7;

    double val = a11_2_float(payload);
    payload += 4;

    sp_json_object_add(json, "timestamp", sp_json_string(timestamp));
    sp_json_object_add(json, "val", sp_json_double(val));

    return 11;    
}

static int fn5_2_json(sp_json_t *json, unsigned char *payload)
{
    int length = 0;
    char timestamp[16];
    tdd_2_string(timestamp, payload);
    payload += 3;
    length += 3;

    sp_json_object_add(json, "timestamp", sp_json_string(timestamp));

    unsigned char m = *payload;
    payload += 1;
    length += 1;

    double total = a13_2_float(payload);
    payload += 4;
    length += 4;
    sp_json_object_add(json, "total", sp_json_double(total));

    sp_json_t *values = sp_json_array_new();
    int i;
    for (i = 0; i < m; i++)
    {
        double val = a13_2_float(payload);
        payload += 4;
        length += 4;
        sp_json_array_add(values, sp_json_double(val));
    }

    sp_json_object_add(json, "val", values);

    return length;
}

static int fn185_2_json(sp_json_t *json, unsigned char *payload)
{
    int length = 0;
    char timestamp[16];
    unsigned char year = payload[2];
    tdd_2_string(timestamp, payload);
    payload += 3;
    length += 3;
        /* ignore */

    char terminal_ts[16];
    tdc_2_string(terminal_ts, payload);
    payload += 5;
    length += 5;
    sp_json_object_add(json, "timestamp", sp_json_string(terminal_ts));

    unsigned char m = *payload;
    payload += 1;
    length += 1;

    double val_max = a23_2_float(payload);
    payload += 3;
    length += 3;
    sp_json_object_add(json, "val_max", sp_json_double(val_max));

    char ts_max[16];
    a17_2_string(ts_max, year, payload);
    payload += 4;
    length += 4;
    sp_json_object_add(json, "ts_max", sp_json_string(ts_max));

    sp_json_t *values = sp_json_array_new();
    int i;
    for (i = 0; i < m; i++)
    {
        double val_i = a23_2_float(payload);
        payload += 3;

        char ts_i[16];
        a17_2_string(ts_i, year, payload);
        payload += 4;

        length += 7;

        sp_json_t *val = sp_json_object_new();
        sp_json_object_add(val, "val", sp_json_double(val_i));
        sp_json_object_add(val, "timestamp", sp_json_string(ts_i));

        sp_json_array_add(values, val);
    }

    sp_json_object_add(json, "val", values);

    return length;
}

//充电桩静态数据
static int fn212_2_json(sp_json_t *json, unsigned char *payload)
{
    int length = 0;
    unsigned char *origin = payload;

    char number[32];
    sp_string_ncopy(number, payload, 20);
    payload += 20;
    sp_json_object_add(json, "serial_number", sp_json_string(number));

    short u_max = bcd2_2_int(payload);
    payload += 2;
    sp_json_object_add(json, "u_max", sp_json_int(u_max));

    short u_min = bcd2_2_int(payload);
    payload += 2;
    sp_json_object_add(json, "u_min", sp_json_int(u_min));

    short i_max = bcd2_2_int(payload);
    payload += 2;
    sp_json_object_add(json, "i_max", sp_json_int(i_max));

    unsigned char m = *payload;
    payload += 1;
    sp_json_object_add(json, "module_count", sp_json_int(m));

    short p = bcd2_2_int(payload);
    payload += 2;
    sp_json_object_add(json, "module_power", sp_json_int(p));

    /* 0-无效，1-一体交流单枪，2-一体交流双枪，3-分体交流单枪，4-一体直流单枪，5-一体直流双枪，6-分体直流单枪 */
    unsigned char hub_type = *payload;
    payload += 1;
    sp_json_object_add(json, "hub_type", sp_json_int(hub_type));
    
    unsigned char location_tag = *payload;
    payload += 1;

    double longtitude = char4_2_float6(payload);
    payload += 4;

    double latitude = char4_2_float6(payload);
    payload += 4;

    if (location_tag & 1)
    {
        //sp_json_object_add(json, "longtitude", sp_json_double(0.0));
        //sp_json_object_add(json, "latitude", sp_json_double(0.0));
    }
    else
    {
        if (location_tag & 2)
        {
            latitude = -latitude;
        }

        if (location_tag & 4)
        {
            longtitude = -longtitude;
        }
        sp_json_object_add(json, "longtitude", sp_json_double(longtitude));
        sp_json_object_add(json, "latitude", sp_json_double(latitude));
    }

    /* 0-A枪，1-B枪，2-C枪 */
    unsigned char gun_priority = *payload;
    payload += 1;
    sp_json_object_add(json, "gun_priority", sp_json_int(gun_priority));

    short ct = bcd2_2_int(payload);
    payload += 2;
    short pt = bcd2_2_int(payload);
    payload += 2;
    sp_json_object_add(json, "ct", sp_json_int(ct));
    sp_json_object_add(json, "pt", sp_json_int(pt));

    /* 0-英科瑞，1-艾默生，2-英飞源，3-中兴，4-华为 */
    unsigned char module_type = *payload;
    payload += 1;
    sp_json_object_add(json, "module_type", sp_json_int(module_type));

    char manufacture_date[16];
    tdd_2_string(manufacture_date, payload);
    payload += 3;
    sp_json_object_add(json, "manufacture_date", sp_json_string(manufacture_date));

    char manufacturer_info[16];
    sp_string_ncopy(manufacturer_info, payload, 4);
    payload += 4;
    sp_json_object_add(json, "manufacturer_info", sp_json_string(manufacturer_info));

    char hw_version[16];
    sp_string_ncopy(hw_version, payload, 4);
    payload += 4;
    sp_json_object_add(json, "hardware_version", sp_json_string(hw_version));

    char sw_version[16];
    sp_string_ncopy(sw_version, payload, 4);
    payload += 4;
    sp_json_object_add(json, "software_version", sp_json_string(sw_version));

    char sw_date[16];
    tdd_2_string(sw_date, payload);
    payload += 4;
    sp_json_object_add(json, "software_date", sp_json_string(sw_date));

    char protocol_version[16];
    sp_string_ncopy(protocol_version, payload, 4);
    payload += 4;
    sp_json_object_add(json, "protocol_version", sp_json_string(protocol_version));

    length = payload - origin + 59;

    return length;
}

//远程充值，桩请求，主站应答
static int fn213_2_json(sp_json_t *json, unsigned char *payload)
{
    int length = 0;

    unsigned char *origin = payload;

    payload += 46;

    length = payload - origin;

    app_log_fatal("fatal error recv F213");
    
    return length;
}

//桩查询未结算记录，桩请求，主站应答
static int fn214_2_json(sp_json_t *json, unsigned char *payload)
{
    int length = 0;

    unsigned char *origin = payload;

    payload += 30;

    length = payload - origin;

    app_log_fatal("fatal error recv F214");

    return length;
}

//已结算记录，桩请求，主站应答
static int fn215_2_json(sp_json_t *json, unsigned char *payload)
{
    int length = 0;

    unsigned char *origin = payload;

    payload += 26;

    length = payload - origin;

    app_log_fatal("fatal error recv F215");
    
    return length;
}

//充电过程报文
static int fn216_2_json(sp_json_t *json, unsigned char *payload)
{
    int length = 0;
    unsigned char *origin = payload;

    payload += 16;

    payload += 2;

    payload += 2;

    unsigned char n = *payload;
    payload += 1;

    int i;
    for (i = 0; i < n; i++)
    {
        payload += 17;
    }

    length = payload - origin;

    return length;
}
//充电起停事件
static int fn217_2_json(sp_json_t *json, unsigned char *payload)
{
    int length = 0;
    unsigned char *origin = payload;

    unsigned char gun = *payload;
    payload += 1;
    sp_json_object_add(json, "gun", sp_json_int(gun));

    unsigned char charge_type = *payload;
    payload += 1;
    if (charge_type == 1)
    {
        sp_json_object_add(json, "account_type", sp_json_string("app"));
    }
    else if (charge_type == 2)
    {
        sp_json_object_add(json, "account_type", sp_json_string("card"));
    }
    else
    {
        sp_json_object_add(json, "account_type", sp_json_string("unknown"));
    }

    char account[32];
    bcd8_2_string(account, payload);
    payload += 8;
    sp_json_object_add(json, "account", sp_json_string(account));

    unsigned char tag = *payload;
    payload += 1;
    if (tag == 0)
    {
        sp_json_object_add(json, "event", sp_json_string("stop"));
    }
    else if (tag == 1)
    {
        sp_json_object_add(json, "event", sp_json_string("start"));
    }
    else if (tag == 2)
    {
        sp_json_object_add(json, "event", sp_json_string("pause"));
    }
    else if (tag == 3)
    {
        sp_json_object_add(json, "event", sp_json_string("resume"));
    }
    else
    {
        sp_json_object_add(json, "event", sp_json_string("unknown"));
    }

    unsigned char result = *payload;
    payload += 1;
    if (result == 0)
    {
        sp_json_object_add(json, "result", sp_json_int(0));
    }
    else
    {
        sp_json_object_add(json, "result", sp_json_int(-1));
    }

    unsigned char reason = *payload;
    payload += 1;

    unsigned char charge_control_type = *payload;
    payload += 1;
    

    double charge_control_param = a11_2_float(payload);
    payload += 4;

    sp_json_t *charge_control = sp_json_object_new();
    sp_json_object_add(json, "charge_control", charge_control);
    sp_json_object_add(charge_control, "val", sp_json_double(charge_control_param));
    if (charge_control_type & 0x80)
    {
        sp_json_object_add(charge_control, "startup", sp_json_string("timing"));
    }
    else
    {
        sp_json_object_add(charge_control, "startup", sp_json_string("immediate"));
    }

    if (charge_control_type & 0x40)
    {
        sp_json_object_add(charge_control, "control", sp_json_string("24v"));
    }
    else
    {
        sp_json_object_add(charge_control, "control", sp_json_string("12v"));
    }

    unsigned char halt = charge_control_type & 0x3f;
    sp_json_object_add(charge_control, "halt", sp_json_string(app_halt_condition_string(halt)));


    char timestamp[16];
    bcd6_2_string(timestamp, payload);
    payload += 6;
    sp_json_object_add(json, "timestamp", sp_json_string(timestamp));

    length = payload - origin;

    return length;
}
//握手事件
static int fn218_2_json(sp_json_t *json, unsigned char *payload)
{
    int length = 0;
    unsigned char *origin = payload;

    unsigned char gun = *payload;
    payload += 1;
    sp_json_object_add(json, "gun", sp_json_int(gun));

    unsigned char result = *payload;
    payload += 1;
    if (result == 0xaa)
    {
        sp_json_object_add(json, "result", sp_json_int(0));
    }
    else
    {
        sp_json_object_add(json, "result", sp_json_int(-1));
    }

    //bms
    int bms_version = (payload[2] << 16) | (payload[1] << 8) | payload[0];
    payload += 3;
    sp_json_object_add(json, "bms_version", sp_json_int(bms_version));

    //b type
    unsigned char battery_type = *payload;
    payload += 1;
    sp_json_object_add(json, "battery_type", sp_json_int(battery_type));

    double battery_capacity = ((payload[1] << 8) | payload[0]) / 10.0;
    payload += 2;
    sp_json_object_add(json, "battery_capacity", sp_json_double(battery_capacity));

    double battery_voltage = ((payload[1] << 8) | payload[0]) / 10.0;
    payload += 2;
    sp_json_object_add(json, "bettery_voltage", sp_json_double(battery_voltage));

    //battery factory code
    char battery_factory_code[8];
    sp_string_ncopy(battery_factory_code, payload, 4);
    payload += 4;
    sp_json_object_add(json, "battery_factory_code", sp_json_string(battery_factory_code));

    //battery group index
    payload += 4;

    //battery date
    char battery_manufacture_date[16];
    tdd_2_string(battery_manufacture_date, payload);
    payload += 3;
    sp_json_object_add(json, "battery_manufacture_date", sp_json_string(battery_manufacture_date));

    //battery used count
    int battery_charged_count = (payload[2] << 16) | (payload[1] << 8) | payload[0];
    payload += 3;
    sp_json_object_add(json, "battery_charged_count", sp_json_int(battery_charged_count));

    //battery owner tag
    unsigned char owner_tag = *payload;
    payload += 1;
    sp_json_object_add(json, "battery_owner_tag", sp_json_int(owner_tag));

    char vin[32];
    sp_string_ncopy(vin, payload, 17);
    payload += 17;
    sp_json_object_add(json, "vin", sp_json_string(vin));

    //reserved
    payload += 1;

    length = payload - origin;

    return length;
}
//充电配置参数
static int fn219_2_json(sp_json_t *json, unsigned char *payload)
{
    int length = 0;
    unsigned char *origin = payload;

    
    payload += 2;
    payload += 27;

    length = payload - origin;

    return length;
}
//中止充电事件
static int fn220_2_json(sp_json_t *json, unsigned char *payload)
{
    int length = 0;

    unsigned char *origin = payload;
    length = payload - origin;

    unsigned char gun = *payload;
    payload += 1;
    sp_json_object_add(json, "gun", sp_json_int(gun));

    //reason
    unsigned char bms_halt_reason = bcd_2_int(*payload);
    payload += 1;

    short bms_halt_fault_reason = bcd2_2_int(payload);
    payload += 2;

    unsigned char bms_halt_error_reason = bcd_2_int(*payload);
    payload += 1;

    unsigned char charger_halt_reason = bcd_2_int(payload);
    payload += 1;

    short charger_halt_fault_reason = bcd2_2_int(payload);
    payload += 2;

    unsigned char charger_halt_error_reason = bcd_2_int(*payload);
    payload += 1;

    sp_json_object_add(json, "bms_halt_reason", sp_json_int(bms_halt_reason));
    sp_json_object_add(json, "bms_halt_fault_reason", sp_json_int(bms_halt_fault_reason));
    sp_json_object_add(json, "bms_halt_error_reason", sp_json_int(bms_halt_error_reason));
    sp_json_object_add(json, "charger_halt_reason", sp_json_int(charger_halt_reason));
    sp_json_object_add(json, "charger_halt_fault_reason", sp_json_int(charger_halt_fault_reason));
    sp_json_object_add(json, "charger_halt_error_reason", sp_json_int(charger_halt_error_reason));

    double money = a11_2_float(payload);
    payload += 4;
    sp_json_object_add(json, "money", sp_json_double(money));

    char vin[32];
    sp_string_ncopy(vin, payload, 17);
    payload += 17;
    sp_json_object_add(json, "vin", sp_json_string(vin));

    unsigned char soc = bcd_2_int(*payload);
    payload += 1;
    sp_json_object_add(json, "soc", sp_json_int(soc));

    double output_power = char4_2_float2(payload);
    payload += 4;
    sp_json_object_add(json, "output_power", sp_json_double(output_power));

    payload += 20;

    length = payload - origin;
    return length;
}

//充电鉴权数据，桩请求，主站应答
static int fn221_2_json(sp_json_t *json, unsigned char *payload)
{
    int length = 0;
    unsigned char *origin = payload;

    payload += 40;

    length = payload - origin;

    app_log_fatal("fatal error recv F221");
    
    return length;
}

//充电实时数据V1.0
static int fn222_2_json(sp_json_t *json, unsigned char *payload)
{

}
//充电实时数据V1.5
static int fn223_2_json(sp_json_t *json, unsigned char *payload)
{

}
//充电交易记录
static int fn224_2_json(sp_json_t *json, unsigned char *payload)
{
    int length = 0;
    unsigned char *origin = payload;

    char trade_no[64];
    bcd16_2_string(trade_no, payload);
    payload += 16;
    sp_json_object_add(json, "trade_no", sp_json_string(trade_no));

    char account[32];
    bcd8_2_string(account, payload);
    payload += 8;
    sp_json_object_add(json, "account", sp_json_string(account));

    char card_no[32];
    bcd8_2_string(card_no, payload);
    payload += 8;
    sp_json_object_add(json, "card_no", sp_json_string(card_no));

    unsigned char business_type = *payload;
    payload += 1;
    sp_json_object_add(json, "business_type", sp_json_string(app_business_type_string(business_type)));

    unsigned char gun = *payload;
    payload += 1;
    sp_json_object_add(json, "gun", sp_json_int(gun));

    unsigned char time_segment_tag = *payload;
    payload += 1;
    
    char startup_timestamp[16];
    bcd6_2_string(startup_timestamp, payload);
    payload += 6;
    sp_json_object_add(json, "timestamp_startup", sp_json_string(startup_timestamp));

    char halt_timestamp[16];
    bcd6_2_string(halt_timestamp, payload);
    payload += 6;
    sp_json_object_add(json, "timestamp_halt", sp_json_string(halt_timestamp));

    double startup_total = a11_2_float(payload);
    payload += 4;
    sp_json_object_add(json, "total_startup", sp_json_double(startup_total));

    double halt_total = a11_2_float(payload);
    payload += 4;
    sp_json_object_add(json, "total_halt", sp_json_double(halt_total));

    double power_total = a11_2_float(payload);
    payload += 4;
    sp_json_object_add(json, "total_power", sp_json_double(power_total));

    double money = a11_2_float(payload);
    payload += 4;
    sp_json_object_add(json, "total_money", sp_json_double(money));

    //top peak flat bottom
    double top_startup = a11_2_float(payload);
    payload += 4;

    double top_halt = a11_2_float(payload);
    payload += 4;

    double top_power = a11_2_float(payload);
    payload += 4;

    double top_price = a11_2_float(payload);
    payload += 4;

    double top_money = a11_2_float(payload);
    payload += 4;

    double peak_startup = a11_2_float(payload);
    payload += 4;

    double peak_halt = a11_2_float(payload);
    payload += 4;

    double peak_power = a11_2_float(payload);
    payload += 4;

    double peak_price = a11_2_float(payload);
    payload += 4;

    double peak_money = a11_2_float(payload);
    payload += 4;

    double flat_startup = a11_2_float(payload);
    payload += 4;

    double flat_halt = a11_2_float(payload);
    payload += 4;

    double flat_power = a11_2_float(payload);
    payload += 4;

    double flat_price = a11_2_float(payload);
    payload += 4;

    double flat_money = a11_2_float(payload);
    payload += 4;

    double bottom_startup = a11_2_float(payload);
    payload += 4;

    double bottom_halt = a11_2_float(payload);
    payload += 4;

    double bottom_power = a11_2_float(payload);
    payload += 4;

    double bottom_price = a11_2_float(payload);
    payload += 4;

    double bottom_money = a11_2_float(payload);
    payload += 4;

    double balance = a11_2_float(payload);
    payload += 4;

    sp_json_object_add(json, "top_startup", sp_json_double(top_startup));
    sp_json_object_add(json, "top_halt", sp_json_double(top_halt));
    sp_json_object_add(json, "top_power", sp_json_double(top_power));
    sp_json_object_add(json, "top_price", sp_json_double(top_price));
    sp_json_object_add(json, "top_money", sp_json_double(top_money));

    sp_json_object_add(json, "peak_startup", sp_json_double(peak_startup));
    sp_json_object_add(json, "peak_halt", sp_json_double(peak_halt));
    sp_json_object_add(json, "peak_power", sp_json_double(peak_power));
    sp_json_object_add(json, "peak_price", sp_json_double(peak_price));
    sp_json_object_add(json, "peak_money", sp_json_double(peak_money));

    sp_json_object_add(json, "flat_startup", sp_json_double(flat_startup));
    sp_json_object_add(json, "flat_halt", sp_json_double(flat_halt));
    sp_json_object_add(json, "flat_power", sp_json_double(flat_power));
    sp_json_object_add(json, "flat_price", sp_json_double(flat_price));
    sp_json_object_add(json, "flat_money", sp_json_double(flat_money));

    sp_json_object_add(json, "bottom_startup", sp_json_double(bottom_startup));
    sp_json_object_add(json, "bottom_halt", sp_json_double(bottom_halt));
    sp_json_object_add(json, "bottom_power", sp_json_double(bottom_power));
    sp_json_object_add(json, "bottom_price", sp_json_double(bottom_price));
    sp_json_object_add(json, "bottom_money", sp_json_double(bottom_money));

    unsigned char trade_tag = *payload;
    payload += 1;

    char vin[32];
    sp_string_ncopy(vin, payload, 17);
    payload += 17;
    sp_json_object_add(json, "vin", sp_json_string(vin));

    //reserve
    payload += 13;

    unsigned char balance_tag = *payload;
    payload += 1;

    unsigned char startup_soc = *payload;
    payload += 1;

    unsigned char halt_soc = *payload;
    payload += 1;

    sp_json_object_add(json, "soc_startup", sp_json_int(startup_soc));
    sp_json_object_add(json, "soc_halt", sp_json_int(halt_soc));

    short err = *payload;
    payload += 2;

    length = payload - origin;

    return length;
}

//充电实时数据V2.0
static int fn227_2_json(sp_json_t *json, unsigned char *payload)
{
    int length = 0;
    unsigned char *origin = payload;

    unsigned char gun_a = *payload;
    payload += 1;

    unsigned char gun_b = *payload;
    payload += 1;

    unsigned char gun_status[2];
    gun_status[0] = gun_a;
    gun_status[1] = gun_b;

    sp_json_t *fault = sp_json_array_new();
    sp_json_object_add(json, "fault", fault);
    payload += 40;

    unsigned char charger_temp = *payload;
    payload += 1;
    sp_json_object_add(json, "charger_temperature", sp_json_int(charger_temp));

    unsigned char battery_temp = *payload;
    payload += 1;
    sp_json_object_add(json, "battery_temperature", sp_json_int(battery_temp));

    sp_json_t *modules = sp_json_array_new();
    sp_json_object_add(json, "modules", modules);
    int i;
    for (i = 0; i < 32; i++)
    {
        short module_status = (payload[1] << 8) | payload[0];
        sp_json_array_add(modules, sp_json_int(module_status));
        payload += 2;
    }

    double input_a_u = a7_2_float(payload);
    payload += 2;
    sp_json_object_add(json, "input_a_u", sp_json_double(input_a_u));

    double input_b_u = a7_2_float(payload);
    payload += 2;
    sp_json_object_add(json, "input_b_u", sp_json_double(input_b_u));

    double input_c_u = a7_2_float(payload);
    payload += 2;
    sp_json_object_add(json, "input_c_u", sp_json_double(input_c_u));

    double input_a_i = a25_2_float(payload);
    payload += 3;
    sp_json_object_add(json, "input_a_i", sp_json_double(input_a_i));

    double input_b_i = a25_2_float(payload);
    payload += 3;
    sp_json_object_add(json, "input_b_i", sp_json_double(input_b_i));

    double input_c_i = a25_2_float(payload);
    payload += 3;
    sp_json_object_add(json, "input_c_i", sp_json_double(input_c_i));

    double input_active_power = char4_2_float3(payload);
    payload += 4;
    sp_json_object_add(json, "input_active_power", sp_json_double(input_active_power));

    double input_reactive_power = char4_2_float3(payload);
    payload += 4;
    sp_json_object_add(json, "input_reactive_power", sp_json_double(input_reactive_power));

    double input_power_factor = char2_2_float2(payload);
    payload += 2;
    sp_json_object_add(json, "input_power_factor", sp_json_double(input_power_factor));

    double price = char4_2_float5(payload);
    payload += 4;
    sp_json_object_add(json, "price", sp_json_double(price));

    sp_json_t *guns = sp_json_array_new();
    sp_json_object_add(json, "guns", guns);

    for (i = 0; i < 2; i++)
    {
        sp_json_t *gun = sp_json_object_new();
        sp_json_array_add(guns, gun);

        if (gun_status[i] == 0xff)
        {
            continue;
        }

        sp_json_object_add(gun, "gun_status", sp_json_int(gun_status[i]));

        unsigned char switch_status = *payload;
        payload += 1;
        sp_json_object_add(gun, "switch_status", sp_json_int(switch_status));

        unsigned char business_type = *payload;
        payload += 1;
        sp_json_object_add(gun, "business_type", sp_json_int(business_type));

        double output_u = a7_2_float(payload);
        payload += 2;
        sp_json_object_add(gun, "output_u", sp_json_double(output_u));

        double bms_u = a7_2_float(payload);
        payload += 2;
        sp_json_object_add(gun, "bms_u", sp_json_double(bms_u));

        double bms_detect_u = a7_2_float(payload);
        payload += 2;
        sp_json_object_add(gun, "bms_detect_u", sp_json_double(bms_detect_u));

        double output_i = a25_2_float(payload);
        payload += 3;
        sp_json_object_add(gun, "output_i", sp_json_double(output_i));

        double bms_i = a25_2_float(payload);
        payload += 3;
        sp_json_object_add(gun, "bms_i", sp_json_double(bms_i));

        double bms_detect_i = a25_2_float(payload);
        payload += 3;
        sp_json_object_add(gun, "bms_detect_i", sp_json_double(bms_detect_i));

        double output_power = char4_2_float3(payload);
        payload += 4;
        sp_json_object_add(gun, "output_power", sp_json_double(output_power));

        unsigned char relay = *payload;
        payload += 1;
        sp_json_object_add(gun, "relay", sp_json_int(relay));

        double charged_power = a11_2_float(payload);
        payload += 4;
        sp_json_object_add(gun, "power_charged", sp_json_double(charged_power));

        short charged_time = (payload[1] << 8) | payload[0];
        payload += 2;
        sp_json_object_add(gun, "time_charged", sp_json_double(charged_time));
        
        double charged_money = a11_2_float(payload);
        payload += 4;
        sp_json_object_add(gun, "money_charged", sp_json_double(charged_money));

        short time_to_charge = (payload[1] << 8) | payload[0];
        payload += 2;
        sp_json_object_add(gun, "time_to_charge", sp_json_int(time_to_charge));

        double active_power = a11_2_float(payload);
        payload += 4;
        sp_json_object_add(gun, "active_power", sp_json_double(active_power));

        double reactive_power = a11_2_float(payload);
        payload += 4;
        sp_json_object_add(gun, "reactive_power", sp_json_double(reactive_power));

        char account[32];
        bcd16_2_string(account, payload);
        payload += 8;
        sp_json_object_add(gun, "account", sp_json_string(account));

        char card_no[32];
        bcd16_2_string(card_no, payload);
        payload += 8;
        sp_json_object_add(gun, "card_number", sp_json_string(card_no));

        unsigned char soc_startup = *payload;
        payload += 1;
        sp_json_object_add(gun, "soc_startup", sp_json_int(soc_startup));

        unsigned char soc_current = *payload;
        payload += 1;
        sp_json_object_add(gun, "soc_current", sp_json_int(soc_current));

        unsigned char battery_type = *payload;
        payload += 1;
        sp_json_object_add(gun, "battery_type", sp_json_int(battery_type));

        unsigned char dc_positive_temperature = *payload;
        payload += 1;
        sp_json_object_add(gun, "dc_positive_temperature", sp_json_int(dc_positive_temperature));

        double u_min = a6_2_float(payload);
        payload += 2;
        sp_json_object_add(gun, "u_min", sp_json_double(u_min));

        double u_max = a6_2_float(payload);
        payload += 2;
        sp_json_object_add(gun, "u_max", sp_json_double(u_max));

        byte dc_negative_temperature = *payload;
        payload += 1;
        sp_json_object_add(gun, "dc_negative_temperature", sp_json_int(dc_negative_temperature));

        //reserved
        payload += 1;

        char vin[17];
        sp_string_ncopy(vin, payload, 17);
        payload += 17;
        sp_json_object_add(gun, "vin", sp_json_string(vin));

        //reserved
        payload += 15;

        byte charge_mode = *payload;
        payload += 1;
        sp_json_object_add(gun, "charge_mode", sp_json_int(charge_mode));

        short u_max_battery = bcd2_2_int(payload);
        payload += 2;
        sp_json_object_add(gun, "u_max_battery", sp_json_int(u_max_battery));

        byte power_max_battery_temperature = bcd_2_int(*payload);
        payload += 1;
        sp_json_object_add(gun, "power_max_battery_temperature", sp_json_int(power_max_battery_temperature));

        byte temperature_max_detection = bcd_2_int(*payload);
        payload += 1;
        sp_json_object_add(gun, "temperature_max_detection", sp_json_int(temperature_max_detection));

        byte power_min_battery_temperature = bcd_2_int(*payload);
        payload += 1;
        sp_json_object_add(gun, "power_min_battery_temperature", sp_json_int(power_min_battery_temperature));

        byte temperature_min_detection = bcd_2_int(*payload);
        payload += 1;
        sp_json_object_add(gun, "temperature_min_detection", sp_json_int(temperature_min_detection));
    }

    length = payload - origin;

    return length;
}

unsigned int frame376_address_hash(frame_376_address_t *addr)
{
    return (addr->a1 << 16) | addr->a2;
}
