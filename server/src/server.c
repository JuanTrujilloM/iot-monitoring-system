/*
 * server.c - Gestión de sockets y manejo de clientes concurrentes
 *
 * Responsabilidades:
 *   - Crear y configurar el socket TCP (SOCK_STREAM)
 *   - Aceptar conexiones entrantes de sensores y operadores
 *   - Crear un hilo (pthread) por cada cliente conectado
 *   - Despachar mensajes al parser del protocolo
 */

#include "../include/server.h"
#include "../include/logger.h"
#include "../include/protocol.h"

/* TODO: implementar server_init(port) */
/* TODO: implementar server_run()      */
/* TODO: implementar client_handler(void *arg) - función de hilo */
/* TODO: implementar server_shutdown() */
