#include "auth_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

int auth_client_verify(const char* username, const char* password, char* role_buffer, int buffer_size) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    // Declaración de variables que faltaban
    char request[256];
    char buffer[1024];
    int valread;
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) return -1;

    // Resolución del nombre del servicio en la red de Docker
    server = gethostbyname("auth-service");
    if (server == NULL) {
        close(sock);
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(9000);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sock);
        return -1;
    }

    // Formatear y enviar la petición
    snprintf(request, sizeof(request), "%s:%s\n", username, password);
    send(sock, request, strlen(request), 0);

    // Leer la respuesta
    valread = read(sock, buffer, sizeof(buffer) - 1);
    close(sock);

    if (valread > 0) {
        buffer[valread] = '\0';
        // Verificar si la respuesta empieza con ROLE:
        if (strncmp(buffer, "ROLE:", 5) == 0) {
            strncpy(role_buffer, buffer + 5, buffer_size - 1);
            role_buffer[buffer_size - 1] = '\0';
            
            // Limpiar saltos de línea al final
            char* newline = strpbrk(role_buffer, "\r\n");
            if (newline) *newline = '\0';
            
            return 1; // Éxito
        }
    }
    
    return 0; // Fallo en la autenticación o respuesta inválida
}