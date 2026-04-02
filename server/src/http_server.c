/*
 * http_server.c - Servidor HTTP básico (interfaz web de monitoreo)
 *
 * Corre en un hilo separado en un puerto distinto al del protocolo principal.
 * Soporta únicamente peticiones GET.
 *
 * Rutas disponibles:
 *   GET /          → página de login / estado general
 *   GET /sensors   → lista de sensores activos (HTML o JSON)
 *   GET /status    → estado del servidor (uptime, clientes conectados)
 *
 * Debe:
 *   - Parsear la línea de petición HTTP (método, path, versión)
 *   - Leer cabeceras (al menos Content-Length / Host)
 *   - Responder con código de estado correcto (200, 404, 405)
 *   - Incluir cabeceras mínimas: Content-Type, Content-Length
 */

#include "../include/server.h"
#include "../include/sensor_manager.h"
#include "../include/logger.h"

/* TODO: implementar http_server_start(int port)              */
/* TODO: implementar http_handle_client(int client_fd)        */
/* TODO: implementar http_parse_request(...)                  */
/* TODO: implementar http_send_response(int fd, int code, ...) */
