#ifndef PROTOCOL_H
#define PROTOCOL_H

#define MAX_BUFFER_SIZE  1024
#define MAX_ARGS           8

typedef enum {
    CMD_UNKNOWN = 0,
    CMD_REGISTER_SENSOR,
    CMD_MEASUREMENT,
    CMD_LOGIN,
    CMD_GET_SENSORS,
    CMD_GET_STATUS,
    CMD_GET_ALERTS,
    CMD_PING,
    CMD_OPERATOR_IDENTIFY
} CommandType;

typedef struct {
    CommandType type;
    char* args[MAX_ARGS];           // pointers within the buffer
    int argc;
    char raw_buffer[MAX_BUFFER_SIZE]; 
} ParsedMessage_t;

/* Parse received message */
int protocol_parse(const char* buffer, int bytes_received, ParsedMessage_t* msg);

/* Construir respuestas (devuelven puntero a buffer interno) */
const char* protocol_build_ok(const char* details);
const char* protocol_build_error(const char* code, const char* desc);
const char* protocol_build_alert(const char* sensor_id, const char* message);
const char* protocol_build_sensors_list(const char* list_data);
const char* protocol_build_alerts_list(const char* alerts_data);
const char* protocol_build_status(const char* status_text);

/* Detect if it is an HTTP request (for the web interface) */
int protocol_is_http(const char* buffer);

#endif
