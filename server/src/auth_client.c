#include "auth_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

int auth_client_verify(const char* username, const char* password, char* role_buffer, int buffer_size) {
    int sock = -1;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int connected = 0;
    
    // Declaration of missing variables
    char request[256];
    char buffer[1024];
    int valread;
    
    // Try first naming Docker and then localhost for local environment.
    {
        const char* auth_hosts[] = {"auth-service", "127.0.0.1"};
        size_t i;

        for (i = 0; i < sizeof(auth_hosts) / sizeof(auth_hosts[0]); i++) {
            if (sock >= 0) {
                close(sock);
                sock = -1;
            }

            sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock < 0) {
                return -1;
            }

            memset(&serv_addr, 0, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(9000);

            if (strcmp(auth_hosts[i], "127.0.0.1") == 0) {
                if (inet_pton(AF_INET, auth_hosts[i], &serv_addr.sin_addr) != 1) {
                    continue;
                }
            } else {
                server = gethostbyname(auth_hosts[i]);
                if (server == NULL || server->h_addr_list == NULL || server->h_addr_list[0] == NULL) {
                    continue;
                }
                memcpy(&serv_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
            }

            if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == 0) {
                connected = 1;
                break;
            }
        }
    }

    if (!connected) {
        if (sock >= 0) {
            close(sock);
        }
        return -1;
    }

    // Format and send the request
    snprintf(request, sizeof(request), "%s:%s\n", username, password);
    send(sock, request, strlen(request), 0);

    // Read the answer
    valread = read(sock, buffer, sizeof(buffer) - 1);
    close(sock);

    if (valread > 0) {
        buffer[valread] = '\0';
        // Check if the answer starts with ROLE:
        if (strncmp(buffer, "ROLE:", 5) == 0) {
            strncpy(role_buffer, buffer + 5, buffer_size - 1);
            role_buffer[buffer_size - 1] = '\0';
            
            // Clean up line breaks at the end
            char* newline = strpbrk(role_buffer, "\r\n");
            if (newline) *newline = '\0';
            
            return 1; // Success
        }
    }
    
    return 0; // Authentication failure or invalid response
}