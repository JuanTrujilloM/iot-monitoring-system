/*
 * protocol.c - Parser e intérprete del protocolo de aplicación (texto)
 *
 * Formato general de mensajes (ver docs/protocol-spec.md):
 *   COMANDO [args...]\n
 *
 * Mensajes soportados:
 *   REGISTER SENSOR <id> <tipo>
 *   DATA <sensor_id> <valor> <unidad>
 *   AUTH <usuario> <password>
 *   LIST SENSORS
 *   LIST ALERTS
 *   GET MEASUREMENTS <sensor_id> [n]
 *   PING
 *
 * Respuestas del servidor:
 *   OK [payload]
 *   ERROR <codigo> <descripcion>
 *   ALERT <sensor_id> <tipo_alerta> <valor>
 */

#include "../include/protocol.h"

/* TODO: implementar parse_message(char *raw, Message *out)    */
/* TODO: implementar handle_message(int client_fd, Message *m) */
/* TODO: implementar send_response(int fd, const char *resp)   */
