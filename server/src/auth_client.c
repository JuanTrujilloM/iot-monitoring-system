/*
 * auth_client.c - Consulta al servicio externo de autenticación
 *
 * El servidor NO almacena usuarios localmente.
 * Para autenticar, abre una conexión TCP al auth-service y envía:
 *   AUTH_CHECK <usuario> <password>\n
 * Respuesta esperada:
 *   AUTH_OK <rol>\n      (rol: operator | admin)
 *   AUTH_FAIL <motivo>\n
 *
 * El hostname del auth-service se resuelve por DNS (getaddrinfo),
 * NUNCA por IP hardcodeada.
 */

#include "../include/server.h"

/* TODO: implementar auth_check(const char *user, const char *pass, char *role_out) */
/*       - usar getaddrinfo() para resolver AUTH_SERVICE_HOST                        */
/*       - manejar fallo de resolución sin terminar el proceso                       */
