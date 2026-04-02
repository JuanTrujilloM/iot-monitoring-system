#ifndef PROTOCOL_H
#define PROTOCOL_H

/* Tipos de mensaje del protocolo */
typedef enum {
    MSG_REGISTER_SENSOR,
    MSG_DATA,
    MSG_AUTH,
    MSG_LIST_SENSORS,
    MSG_LIST_ALERTS,
    MSG_GET_MEASUREMENTS,
    MSG_PING,
    MSG_UNKNOWN
} MessageType;

/* TODO: definir struct Message { MessageType type; char args[][]; int argc; } */
/* TODO: declarar parse_message, handle_message, send_response                 */

#endif /* PROTOCOL_H */
