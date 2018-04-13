

typedef struct
{
    char *operator_id; //9
    char *operator_name; //64
    char *operator_tel1; //32
    char *operator_tel2; //32
    char *operator_reg_address; //64
    char *operator_note;//255
} operator_info_t;

typedef struct
{
    char *station_id; //20
    char *operator_id; //9
    char *equipment_owner_id;//9
    char *station_name;//50
    char *country_code;//2

    char *area_code;//20

    char *address;//50

    char *station_tel;//30 not necessary
    char *service_tel;//30 like 400

    int station_type;

    int station_status;

    int park_nums; //default 0

    double station_lng;
    double station_lat;

    char *site_guide;//100

    int construction;

    binary pic; //not necessary

    char *match_cars;//100 not

    char *park_info;//100 not 

    char *busine_hours;//100 not


    char *electricity_fee;//256 not
    char *service_fee;//100 not
    char *park_fee;//100 not

    char *payment;//20 not

    int support_order;//not

    char *remark;//100 not

    equipment_info_t *infos;//

    
    
} station_info_t;

typedef struct
{
    char *equipment_id;//23
    char *manufacture_id;//9 not

    char *manufacture_name;//30 not
    char *equipment_model;//20 not

    char *production_date;//10 not

    int equipment_type;

    connector_info_t *infos;

    double lng;//not
    double lat;//not

    float power;//KW

    char *name;//30 not
} equipment_info_t;

typedef struct
{
    char *connector_id;//26
    char *name;//20 not

    int type;

    int voltage_upper_limits;
    int voltage_lower_limits;

    int current;//e ding dianliu

    float power;

    char *park_no;//10 not

    int national_standard;
} connector_info_t;

typedef struct
{
    char *connector_id;//26

    int status;

    int park_status;//not

    int lock_status;//not
} connector_status_t;

typedef struct
{
    char *station_id;
    connector_status_t *infos;
} station_status_info_t;

typedef struct
{
    char *station_id;//20

    char *start_time;//yyyy-MM-dd 10
    char *end_time;//10

    float station_electricity;

    equipment_stats_info_t *infos;
} station_stats_info_t;

typedef struct
{
    char *equipment_id;
    float equipment_electricity;

    connector_stats_info_t *infos;
} equipment_stats_info_t;

typedef struct
{
    char *connector_id;//26

    float connector_electricity;
} connector_stats_info_t;

typedef struct
{
    
} business_policy_t;


control
address
data

//F10
typedef struct
{
    short equipment_id, index, unique
    short celiangdian,

    int bitrate,
    int port,

    int protocol_type,

    int address,
} equipment_param_t;

//F221
typedef struct
{
    timestamp_seq,
    card_no,
    int status,
    
} black_list_t;


//F222
typedef struct
{
    model_id, 
    start_time
    end_time,
    status,
    charge_type,


    json: [
        (start_time, charge_price),
        ...
    ]
} charge_model_t;

//F223
typedef struct
{
    int connector,
    int auto_close,

    CT
    PT

    float warn_low_limit,
    float allow_overcharge,

    int enable_card,

    int lock_card,

    char password[3],

    input_low_limit_u,
    input_high_limit_u,
    output_high_limit_u,
    BMS_limit_u,

    
} equipment_running_param_t;

//F31set time

//F47 ORDER

//F48 start/stop

//F49 poweroff ?

//F212  充电桩静态数据

//F213 桩请求充值

//F214 桩查询未结算记录

//F215 桩上报已结算事件

//F216 桩上报充电过程报文

//F217 桩上报充电起停

//F218 桩上报握手事件

//F219 桩上报充电配置参数

//F220 中止充电事件

//F221 桩请求充电鉴权

//F222 桩上报充电实时数据
