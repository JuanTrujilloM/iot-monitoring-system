#include "auth_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int auth_client_verify(const char* username, const char* password, char* role_buffer, int buffer_size) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    char request[512];
    int valread;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(9000);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        close(sock);
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sock);
        return -1;
    }

    snprintf(request, sizeof(request), "%s:%s\n", username, password);
    send(sock, request, strlen(request), 0);

    valread = read(sock, buffer, 1024);
    close(sock);

    if (valread > 0) {
        buffer[valread] = '\0';
        if (strncmp(buffer, "ROLE:", 5) == 0) {
            strncpy(role_buffer, buffer + 5, buffer_size - 1);
            role_buffer[buffer_size - 1] = '\0';
            char* newline = strpbrk(role_buffer, "\r\n");
            if (newline) *newline = '\0';
            return 1;
        }
    }
    return 0;
}