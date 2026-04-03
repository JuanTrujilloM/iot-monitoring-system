#include "server.h"
#include "logger.h"
#include "protocol.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif
#define CLOSE_SOCKET closesocket
#else
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define CLOSE_SOCKET close
#endif
#include <pthread.h>
#include <stdio.h>

#ifdef _WIN32
typedef int socklen_t;
#endif

typedef struct client_context {
	int client_fd;
	char ip[INET_ADDRSTRLEN];
	int port;
} client_context_t;

static void configure_receive_timeout(int client_fd) {
#ifdef _WIN32
	DWORD timeout_ms = 60000;
	if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout_ms, sizeof(timeout_ms)) < 0) {
		logger_error("Failed to configure receive timeout");
	}
#else
	struct timeval tv;

	tv.tv_sec = 60;
	tv.tv_usec = 0;
	if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv)) < 0) {
		logger_error("Failed to configure receive timeout");
	}
#endif
}

static int send_complete_response(int client_fd, const char *response, const char *client_ip, int client_port, const char *received_message) {
	int total_sent = 0;
	int response_length = (int)strlen(response);

	while (total_sent < response_length) {
		int sent = send(client_fd, response + total_sent, response_length - total_sent, 0);

		if (sent <= 0) {
			perror("send");
			logger_event("ERROR", client_ip, client_port, received_message, "Error sending response");
			return -1;
		}

		total_sent += sent;
	}

	return 0;
}

static void *handle_client(void *arg) {
    client_context_t *context = (client_context_t *)arg;
    int client_fd = context->client_fd;
    char client_ip[INET_ADDRSTRLEN];
    int client_port = context->port;
    char buffer[1024];
    int bytes_received;

    snprintf(client_ip, sizeof(client_ip), "%s", context->ip);
    free(context);

    configure_receive_timeout(client_fd);

    logger_event("INFO", client_ip, client_port, "Connection started", "type=unknown");

    while ((bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0) {

        ParsedMessage_t msg;
        const char *response = "ERROR 500 Unknown command\r\n";

        // Parse the message using the protocol
        if (protocol_parse(buffer, bytes_received, &msg) == 0) {

            switch (msg.type) {
                case CMD_PING:
                    response = "PONG\r\n";
                    break;

                case CMD_REGISTER_SENSOR:
                    if (msg.argc >= 2) {
                        // Here, sensor_manager_register(...) will then be called
                        response = protocol_build_ok("SENSOR_REGISTERED");
                    } else {
                        response = protocol_build_error("400", "Missing arguments");
                    }
                    break;

                case CMD_MEASUREMENT:
                    if (msg.argc >= 3) {
                        // Here, sensor_manager_add_measurement(...) will then be called
                        // and to alert_engine_check(...)
                        response = protocol_build_ok("MEASUREMENT_RECEIVED");
                    } else {
                        response = protocol_build_error("400", "Missing arguments");
                    }
                    break;

                case CMD_LOGIN:
                    if (msg.argc >= 2) {
                        // Here, auth_client.c will then be called.
                        response = protocol_build_ok("ROLE_OPERATOR");
                    } else {
                        response = protocol_build_error("401", "Invalid credentials");
                    }
                    break;

                case CMD_GET_SENSORS:
                    // Here, sensor_manager_get_list(...) will then be called
                    response = protocol_build_sensors_list("sensor-temp-001,temp,45.3,°C;sensor-vib-002,vib,12.5,mm/s");
                    break;

                case CMD_GET_STATUS:
                    response = protocol_build_status("5 sensors active - System OK");
                    break;

                default:
                    response = protocol_build_error("404", "Command not found");
                    break;
            }
        }

        // Send response and log
        if (send_complete_response(client_fd, response, client_ip, client_port, msg.raw_buffer) == 0) {
            logger_event("INFO", client_ip, client_port, msg.raw_buffer, response);
        } else {
            break;  // Error sending → Close connection
        }
    }

    // The client disconnected or there was an error
    if (bytes_received == 0) {
        logger_event("INFO", client_ip, client_port, "Client disconnected", "");
    } else {
        logger_event("INFO", client_ip, client_port, "Receive error or timeout", "");
    }

    CLOSE_SOCKET(client_fd);
    return NULL;
}

int start_server(int port, const char *log_file) {

#ifdef _WIN32
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
		perror("WSAStartup");
		return EXIT_FAILURE;
	}
#endif

	int server_fd;
	int reuse_addr = 1;
	struct sockaddr_in server_addr;
	char message[256];

	if (logger_init(log_file) != 0) {
		perror("Failed to open log file");
		return EXIT_FAILURE;
	}

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		perror("socket");
		logger_error("Failed to create server socket");
		logger_close();
#ifdef _WIN32
		WSACleanup();
#endif
		return EXIT_FAILURE;
	}

#ifdef _WIN32
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse_addr, sizeof(reuse_addr)) < 0) {
#else
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)) < 0) {
#endif
		perror("setsockopt(SO_REUSEADDR)");
		logger_error("Failed to configure SO_REUSEADDR");
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons((unsigned short)port);

	if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		perror("bind");
		logger_error("Server bind failed");
		CLOSE_SOCKET(server_fd);
		logger_close();
	#ifdef _WIN32
		WSACleanup();
	#endif
		return EXIT_FAILURE;
	}

	if (listen(server_fd, SOMAXCONN) < 0) {
		perror("listen");
		logger_error("Server listen failed");
		CLOSE_SOCKET(server_fd);
		logger_close();
	#ifdef _WIN32
		WSACleanup();
	#endif
		return EXIT_FAILURE;
	}

	snprintf(message, sizeof(message), "Server listening on port %d", port);
	logger_info(message);

	while (1) {
		int client_fd;
		client_context_t *context;
		pthread_t tid;
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
		char client_ip[INET_ADDRSTRLEN];

		client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
		if (client_fd < 0) {
#ifndef _WIN32
			if (errno == EINTR) {
				continue;
			}
#endif

			perror("accept");
		logger_error("Network error accepting client; server continues");
			continue;
		}

		if (inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip)) == NULL) {
			snprintf(client_ip, sizeof(client_ip), "unknown");
		}

		logger_event("INFO", client_ip, ntohs(client_addr.sin_port), "Incoming connection", "");

		context = (client_context_t *)malloc(sizeof(client_context_t));
		if (context == NULL) {
			logger_error("Error allocating memory for client");
			CLOSE_SOCKET(client_fd);
			continue;
		}

		context->client_fd = client_fd;
		context->port = ntohs(client_addr.sin_port);
		memset(context->ip, 0, sizeof(context->ip));
		snprintf(context->ip, sizeof(context->ip), "%s", client_ip);

		if (pthread_create(&tid, NULL, handle_client, context) != 0) {
			logger_error("Error creating thread");
			CLOSE_SOCKET(client_fd);
			free(context);
			continue;
		}

		pthread_detach(tid);
	}

	CLOSE_SOCKET(server_fd);
	logger_close();
#ifdef _WIN32
	WSACleanup();
#endif
	return EXIT_SUCCESS;
}
