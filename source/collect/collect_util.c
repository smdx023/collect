#include "collect.h"

void app_userdata_report(const char *url, sp_json_t *json)
{
    sp_return_if_fail(json);

    app_log_debug("===> push data to %s", url);

    sp_json_t *headers = sp_json_object_new();
    sp_json_object_add(headers, "Content-Type", sp_json_string("application/json"));

    sp_http_response_t *res = sp_http_post_json(url, headers, 1, json);
    sp_json_free(headers);

    if (res)
    {
        app_log_debug("status_code:%d, body:%s", res->status_code, sp_string_buffer_string(res->raw_body));
    }
    else
    {
        app_log_error("push failed");
    }

    sp_http_response_free(res);
}

void app_print_payload(unsigned char *payload, int length)
{
    int i;
    char buffer[1024];
    sp_string_clear(buffer);
    sp_string_append(buffer, "payload(%d):", length);
    for (i = 0; i < length; i++)
    {
        sp_string_append(buffer, "%02x ", payload[i]);
    }

    sp_string_append(buffer, "%s", "\n");
    printf(buffer);
}

unsigned char app_halt_condition(const char *string)
{
    return 0;
}

char *app_halt_condition_string(unsigned char condition)
{
    if (condition == 1)
    {
        return "power";
    }
    else if (condition == 2)
    {
        return "time";
    }
    else if (condition == 3)
    {
        return "money";
    }
    else if (condition == 4)
    {
        return "full";
    }

    return "unknown";
}

char *app_business_type_string(unsigned char business_type)
{
    if (business_type == 1)
    {
        return "app";
    }
    else if (business_type == 2)
    {
        return "online_card";
    }
    else if (business_type == 3)
    {
        return "offline_card";
    }
    else if (business_type == 4)
    {
        return "without_card";
    }
    else if (business_type == 5)
    {
        return "vin";
    }
    else
    {
        return "unknown";
    }
}

char *app_halt_reason_string(unsigned char reason)
{
    char *result = "unknown";

    switch (reason)
    {
        case 0:
            result = "invalid";
            break;
        case 1:
            result = "startup failed";
            break;
        case 2:
            result = "halt timeout";
            break;
        case 3:
            result = "halt manually";
            break;
        case 4:
            result = "condition fit";
            break;
        case 5:
            result = "halt by car";
            break;
        case 6:
            result = "charger fault";
            break;
        case 7:
            result = "BMS communication timeout";
            break;
        case 8:
            result = "charging 6V";
            break;
        case 9:
            result = "charging 12V";
            break;
        case 10:
            result = "gun communication exception";
            break;
        case 11:
            result = "pre-charge failed";
            break;
        case 12:
            result = "insulation exception";
            break;
        case 13:
            result = "sampling voltage greater than threshold";
            break;
        case 14:
            result = "relay adhesion";
            break;
        case 15:
            result = "excessively high lateral voltage of contactor";
            break;
        case 16:
            result = "DC output overvoltage (over limit)";
            break;
        case 17:
            result = "DC output overcurrent (over limit)";
            break;
        case 18:
            result = "Overvoltage (over limit) of AC input";
            break;
        case 19:
            result = "halt by emergency";
            break;
        case 20:
            result = "halt by app";
            break;
        default:
            result = "other";
            break;
    }

    return result;
}

char *app_gun_status_string(unsigned char status)
{
    char *result = "unknown";

    switch (status)
    {
        case 0:
            result = "standby";
            break;
        case 1:
            result = "working";
            break;
        case 2:
            result = "fault";
            break;
        case 3:
            result = "order";
            break;
        case 4:
            result = "timing";
            break;
        case 5:
            result = "reserved";
            break;
        case 6:
            result = "connected";
            break;
        case 7:
            result = "chargeover";
            break;
        case 0xff:
            result = "invalid";
            break;
        default:
            break;
    }

    return result;
}

task_context_t *app_task_context()
{
    app_context_t *ctx = app_context();
    task_context_t *task_ctx = sp_tls_get(ctx->task_tls);

    return task_ctx;
}

