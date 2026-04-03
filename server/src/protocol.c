#include "protocol.h"
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <ctype.h>

static char resp_buf[512];

int protocol_parse(const char* buffer, int bytes_received, ParsedMessage_t* msg)
{
    if (bytes_received <= 0 || bytes_received >= MAX_BUFFER_SIZE)
        return -1;

    memcpy(msg->raw_buffer, buffer, bytes_received);
    msg->raw_buffer[bytes_received] = '\0';

    // Quitar \r\n finales
    char* p = msg->raw_buffer;
    while (*p) {
        if (*p == '\r' || *p == '\n') { *p = '\0'; break; }
        p++;
    }

    msg->argc = 0;
    msg->type = CMD_UNKNOWN;

    char* token = strtok(msg->raw_buffer, " ");
    if (!token) return 0;

    // Comandos en mayúsculas (case-insensitive)
    if (strcasecmp(token, "REGISTER_SENSOR") == 0)  msg->type = CMD_REGISTER_SENSOR;
    else if (strcasecmp(token, "MEASUREMENT") == 0) msg->type = CMD_MEASUREMENT;
    else if (strcasecmp(token, "LOGIN") == 0)        msg->type = CMD_LOGIN;
    else if (strcasecmp(token, "GET_SENSORS") == 0)  msg->type = CMD_GET_SENSORS;
    else if (strcasecmp(token, "GET_STATUS") == 0)   msg->type = CMD_GET_STATUS;
    else if (strcasecmp(token, "GET_ALERTS") == 0)   msg->type = CMD_GET_ALERTS;
    else if (strcasecmp(token, "PING") == 0)         msg->type = CMD_PING;
    else if (strcasecmp(token, "OPERATOR_IDENTIFY") == 0) msg->type = CMD_OPERATOR_IDENTIFY;

    while ((token = strtok(NULL, " ")) && msg->argc < MAX_ARGS) {
        msg->args[msg->argc++] = token;
    }

    return 0;
}

const char* protocol_build_ok(const char* details)
{
    snprintf(resp_buf, sizeof(resp_buf), "OK %s\r\n", details ? details : "");
    return resp_buf;
}

const char* protocol_build_error(const char* code, const char* desc)
{
    snprintf(resp_buf, sizeof(resp_buf), "ERROR %s %s\r\n", code ? code : "500", desc ? desc : "");
    return resp_buf;
}

const char* protocol_build_alert(const char* sensor_id, const char* message)
{
    snprintf(resp_buf, sizeof(resp_buf), "ALERT %s %s\r\n", sensor_id, message);
    return resp_buf;
}

const char* protocol_build_sensors_list(const char* list_data)
{
    snprintf(resp_buf, sizeof(resp_buf), "SENSORS %s\r\n", list_data ? list_data : "");
    return resp_buf;
}

const char* protocol_build_alerts_list(const char* alerts_data)
{
    snprintf(resp_buf, sizeof(resp_buf), "ALERTS %s\r\n", alerts_data ? alerts_data : "");
    return resp_buf;
}

const char* protocol_build_status(const char* status_text)
{
    snprintf(resp_buf, sizeof(resp_buf), "STATUS %s\r\n", status_text ? status_text : "OK");
    return resp_buf;
}

int protocol_is_http(const char* buffer)
{
    if (!buffer) return 0;
    return (strncmp(buffer, "GET ", 4) == 0 ||
            strncmp(buffer, "POST ", 5) == 0);
}
