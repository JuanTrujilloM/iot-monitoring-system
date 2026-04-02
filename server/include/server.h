#ifndef SERVER_H
#define SERVER_H

#define MAX_CLIENTS     64
#define BUFFER_SIZE     4096
#define HTTP_PORT_OFFSET 1   /* HTTP corre en puerto+1 del principal */

/* Hostname del auth-service (resuelto por DNS, nunca IP) */
#define AUTH_SERVICE_HOST "auth-service"
#define AUTH_SERVICE_PORT 9000

/* TODO: definir struct ClientInfo { int fd; char ip[]; int port; char role[]; } */
/* TODO: declarar server_init, server_run, server_shutdown */
/* TODO: declarar auth_check */

#endif /* SERVER_H */
