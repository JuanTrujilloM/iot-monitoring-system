/*
 * logger.c - Sistema de registro de eventos
 *
 * Cada entrada de log debe contener:
 *   [TIMESTAMP] [NIVEL] [IP_CLIENTE:PUERTO] mensaje
 *
 * Niveles: INFO | WARN | ERROR
 *
 * Los registros se escriben simultáneamente en:
 *   - stdout (consola)
 *   - archivo de logs (ruta pasada como argumento al servidor)
 *
 * Debe ser thread-safe (usar mutex para escritura concurrente).
 */

#include "../include/logger.h"

/* TODO: implementar logger_init(const char *filepath)                          */
/* TODO: implementar log_info(const char *client_ip, int port, const char *msg) */
/* TODO: implementar log_warn(const char *client_ip, int port, const char *msg) */
/* TODO: implementar log_error(const char *client_ip, int port, const char *msg)*/
/* TODO: implementar logger_close()                                             */
