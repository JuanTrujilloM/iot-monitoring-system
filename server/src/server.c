#include "server.h"
#include "logger.h"
#include "protocol.h"
#include "sensor_manager.h"
#include "alert_engine.h"
#include "auth_client.h"

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
	pthread_mutex_t write_mutex;
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

static int operator_clients[100];
static pthread_mutex_t *operator_write_mutexes[100];
static int operator_count = 0;
static pthread_mutex_t operator_mutex = PTHREAD_MUTEX_INITIALIZER;

static void register_operator(int client_fd, pthread_mutex_t *write_mutex)
{
    pthread_mutex_lock(&operator_mutex);
    if (operator_count < 100) {
        operator_clients[operator_count] = client_fd;
        operator_write_mutexes[operator_count] = write_mutex;
        operator_count++;
        logger_info("Operator registered for alerts");
    }
    pthread_mutex_unlock(&operator_mutex);
}

static void remove_operator(int client_fd)
{
    pthread_mutex_lock(&operator_mutex);
    for (int i = 0; i < operator_count; i++) {
        if (operator_clients[i] == client_fd) {
            operator_clients[i] = -1;
            operator_write_mutexes[i] = NULL;
            break;
        }
    }
    pthread_mutex_unlock(&operator_mutex);
}

static void broadcast_alert(const char* alert_message)
{
    pthread_mutex_lock(&operator_mutex);
    for (int i = 0; i < operator_count; i++) {
        if (operator_clients[i] > 0 && operator_write_mutexes[i] != NULL) {
            pthread_mutex_lock(operator_write_mutexes[i]);
            send(operator_clients[i], alert_message, strlen(alert_message), 0);
            pthread_mutex_unlock(operator_write_mutexes[i]);
        }
    }
    pthread_mutex_unlock(&operator_mutex);
}

static void *handle_client(void *arg) {
    client_context_t *context = (client_context_t *)arg;
    int client_fd = context->client_fd;
    char client_ip[INET_ADDRSTRLEN];
    int client_port = context->port;
    char buffer[1024];
    int bytes_received;

    snprintf(client_ip, sizeof(client_ip), "%s", context->ip);
    /* We don't release context here — we need write_mutex for the entire connection */

    configure_receive_timeout(client_fd);

    logger_event("INFO", client_ip, client_port, "Connection started", "type=unknown");

    while ((bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0) {

        ParsedMessage_t msg;
        const char *response = protocol_build_error("404", "Command not found");

        if (protocol_parse(buffer, bytes_received, &msg) == 0) {

            switch (msg.type) {

                case CMD_PING:
                    response = "PONG\r\n";
                    break;

                case CMD_REGISTER_SENSOR:
                    if (msg.argc >= 2) {
                        if (sensor_manager_register(msg.args[1], msg.args[0]) == 0) {
                            response = protocol_build_ok("SENSOR_REGISTERED");
                        } else {
                            response = protocol_build_error("500", "Failed to register sensor");
                        }
                    } else {
                        response = protocol_build_error("400", "Missing arguments");
                    }
                    break;

                case CMD_MEASUREMENT:
					if (msg.argc >= 3) { 
						double value = atof(msg.args[1]);

						if (sensor_manager_add_measurement(msg.args[0], value, msg.args[2]) == 0) {
							response = protocol_build_ok("MEASUREMENT_RECEIVED");

							const char* sensor_type = sensor_manager_get_sensor_type(msg.args[0]);

							if (sensor_type) {
								int alerts_triggered = alert_engine_check_measurement(msg.args[0], sensor_type, value);
								if (alerts_triggered > 0) {
									char alert_msg[256];
									snprintf(alert_msg, sizeof(alert_msg),
											"ALERTA %s,%s,%.2f,%s\r\n",
											msg.args[0], sensor_type, value, msg.args[2]);
									broadcast_alert(alert_msg);
								}
							}
						} else {
							response = protocol_build_error("404", "Sensor not registered");
						}
					} else {
						response = protocol_build_error("400", "Missing arguments");
					}
					break;

                case CMD_LOGIN:
                    if (msg.argc >= 2) {
                        char role[64] = {0};
                        int auth_result = auth_client_verify(msg.args[0], msg.args[1], role, sizeof(role));

                        if (auth_result == 1) {
                            char ok_msg[128];
                            snprintf(ok_msg, sizeof(ok_msg), "ROLE_%s", role);
                            response = protocol_build_ok(ok_msg);
                            register_operator(client_fd, &context->write_mutex);
                            logger_info("Operator successfully logged in and registered for alerts");
                        } else if (auth_result == 0) {
                            response = protocol_build_error("401", "Invalid credentials");
                        } else {
                            response = protocol_build_error("503", "Auth service unavailable");
                        }
                    } else {
                        response = protocol_build_error("400", "Missing arguments");
                    }
                    break;

                case CMD_OPERATOR_IDENTIFY:
                    response = protocol_build_ok("OPERATOR_CONNECTED");
                    logger_event("INFO", client_ip, client_port, "OPERATOR_IDENTIFY", "Operator supervisor connected");
                    break;

                case CMD_GET_SENSORS:
                {
                    char sensors_list[1024] = {0};
                    if (sensor_manager_get_active_sensors(sensors_list, sizeof(sensors_list)) == 0) {
                        response = protocol_build_sensors_list(sensors_list);
                    } else {
                        response = protocol_build_error("500", "Failed to get sensors");
                    }
                    break;
                }

                case CMD_GET_ALERTS:
                {
                    char alerts_list[1024] = {0};
                    if (alert_engine_get_active_alerts(alerts_list, sizeof(alerts_list)) == 0) {
                        response = protocol_build_alerts_list(alerts_list);
                    } else {
                        response = protocol_build_error("500", "Failed to get alerts");
                    }
                    break;
                }

                case CMD_GET_STATUS:
                {
                    char status_text[128];
                    snprintf(status_text, sizeof(status_text), "%d sensors active - System OK", 5);
                    response = protocol_build_status(status_text);
                    break;
                }

                default:
                    response = protocol_build_error("404", "Command not found");
                    break;
            }
        }

        pthread_mutex_lock(&context->write_mutex);
        int send_result = send_complete_response(client_fd, response, client_ip, client_port, msg.raw_buffer);
        pthread_mutex_unlock(&context->write_mutex);
        if (send_result == 0) {
            logger_event("INFO", client_ip, client_port, msg.raw_buffer, response);
        } else {
            break;
        }
    }

    if (bytes_received == 0) {
        logger_event("INFO", client_ip, client_port, "Client disconnected", "");
    } else {
        logger_event("INFO", client_ip, client_port, "Receive error or timeout", "");
    }

	if (bytes_received == 0) {
        logger_event("INFO", client_ip, client_port, "Client disconnected", "");
    } else {
        logger_event("INFO", client_ip, client_port, "Receive error or timeout", "");
    }

    remove_operator(client_fd);
    CLOSE_SOCKET(client_fd);
    pthread_mutex_destroy(&context->write_mutex);
    free(context);
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
		pthread_mutex_init(&context->write_mutex, NULL);

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
