#include "server.h"

#include "logger.h"

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
	int puerto;
} client_context_t;

static void configurar_timeout_recepcion(int client_fd) {
#ifdef _WIN32
	DWORD timeout_ms = 60000;
	if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout_ms, sizeof(timeout_ms)) < 0) {
		logger_error("No se pudo configurar timeout de recepcion");
	}
#else
	struct timeval tv;

	tv.tv_sec = 60;
	tv.tv_usec = 0;
	if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv)) < 0) {
		logger_error("No se pudo configurar timeout de recepcion");
	}
#endif
}

static int enviar_respuesta_completa(int client_fd, const char *respuesta, const char *ip_cliente, int puerto_cliente, const char *mensaje_recibido) {
	int total_enviado = 0;
	int longitud_respuesta = (int)strlen(respuesta);

	while (total_enviado < longitud_respuesta) {
		int enviados = send(client_fd, respuesta + total_enviado, longitud_respuesta - total_enviado, 0);

		if (enviados <= 0) {
			perror("send");
			logger_evento("ERROR", ip_cliente, puerto_cliente, mensaje_recibido, "Error al enviar respuesta");
			return -1;
		}

		total_enviado += enviados;
	}

	return 0;
}

static int procesar_mensaje_cliente(int client_fd, const char *ip_cliente, int puerto_cliente, char *buffer, int bytes_recibidos, const char *respuesta) {
	buffer[bytes_recibidos] = '\0';

	if (enviar_respuesta_completa(client_fd, respuesta, ip_cliente, puerto_cliente, buffer) != 0) {
		return -1;
	}

	logger_evento("INFO", ip_cliente, puerto_cliente, buffer, respuesta);
	return 0;
}

static void *manejar_cliente(void *arg) {
	client_context_t *contexto = (client_context_t *)arg;
	int client_fd = contexto->client_fd;
	char ip_cliente[INET_ADDRSTRLEN];
	int puerto_cliente = contexto->puerto;
	char buffer[1024];
	int bytes_recibidos;
	const char *respuesta = "OK\n";

	snprintf(ip_cliente, sizeof(ip_cliente), "%s", contexto->ip);
	free(contexto);

	configurar_timeout_recepcion(client_fd);

	logger_evento("INFO", ip_cliente, puerto_cliente, "Conexion iniciada", "tipo=desconocido");

	while ((bytes_recibidos = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0) {
		if (procesar_mensaje_cliente(client_fd, ip_cliente, puerto_cliente, buffer, bytes_recibidos, respuesta) != 0) {
			break;
		}
	}

	if (bytes_recibidos == 0) {
		logger_evento("INFO", ip_cliente, puerto_cliente, "Cliente desconectado", "");
	} else if (bytes_recibidos < 0) {
		#ifdef _WIN32
		{
			int error_socket = WSAGetLastError();

			if (error_socket == WSAETIMEDOUT) {
				logger_evento("INFO", ip_cliente, puerto_cliente, "Timeout de recepcion", "");
			} else {
				perror("recv");
				logger_evento("ERROR", ip_cliente, puerto_cliente, "Error en recv", "");
			}
		}
		#else
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			logger_evento("INFO", ip_cliente, puerto_cliente, "Timeout de recepcion", "");
		} else {
			perror("recv");
			logger_evento("ERROR", ip_cliente, puerto_cliente, "Error en recv", "");
		}
		#endif
	}

	if (CLOSE_SOCKET(client_fd) < 0) {
		perror("close(client_fd)");
		logger_error("No se pudo cerrar el socket de cliente en el hilo");
		return NULL;
	}

	return NULL;
}

int iniciar_servidor(int puerto, const char *archivo_logs) {

#ifdef _WIN32
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
		perror("WSAStartup");
		return EXIT_FAILURE;
	}
#endif

	int server_fd;
	int opcion_reuso = 1;
	struct sockaddr_in server_addr;
	char mensaje[256];

	if (logger_init(archivo_logs) != 0) {
		perror("No se pudo abrir archivo de logs");
		return EXIT_FAILURE;
	}

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		perror("socket");
		logger_error("Fallo al crear socket del servidor");
		logger_close();
#ifdef _WIN32
		WSACleanup();
#endif
		return EXIT_FAILURE;
	}

#ifdef _WIN32
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opcion_reuso, sizeof(opcion_reuso)) < 0) {
#else
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opcion_reuso, sizeof(opcion_reuso)) < 0) {
#endif
		perror("setsockopt(SO_REUSEADDR)");
		logger_error("No fue posible configurar SO_REUSEADDR");
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons((unsigned short)puerto);

	if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		perror("bind");
		logger_error("Fallo bind del servidor");
		CLOSE_SOCKET(server_fd);
		logger_close();
	#ifdef _WIN32
		WSACleanup();
	#endif
		return EXIT_FAILURE;
	}

	if (listen(server_fd, SOMAXCONN) < 0) {
		perror("listen");
		logger_error("Fallo listen del servidor");
		CLOSE_SOCKET(server_fd);
		logger_close();
	#ifdef _WIN32
		WSACleanup();
	#endif
		return EXIT_FAILURE;
	}

	snprintf(mensaje, sizeof(mensaje), "Servidor escuchando en puerto %d", puerto);
	logger_info(mensaje);

	while (1) {
		int client_fd;
		client_context_t *contexto;
		pthread_t tid;
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
		char ip_cliente[INET_ADDRSTRLEN];

		client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
		if (client_fd < 0) {
#ifndef _WIN32
			if (errno == EINTR) {
				continue;
			}
#endif

			perror("accept");
			logger_error("Error de red al aceptar cliente; el servidor continua");
			continue;
		}

		if (inet_ntop(AF_INET, &client_addr.sin_addr, ip_cliente, sizeof(ip_cliente)) == NULL) {
			snprintf(ip_cliente, sizeof(ip_cliente), "desconocida");
		}

		logger_evento("INFO", ip_cliente, ntohs(client_addr.sin_port), "Conexion entrante", "");

		contexto = (client_context_t *)malloc(sizeof(client_context_t));
		if (contexto == NULL) {
			logger_error("Error reservando memoria para cliente");
			CLOSE_SOCKET(client_fd);
			continue;
		}

		contexto->client_fd = client_fd;
		contexto->puerto = ntohs(client_addr.sin_port);
		memset(contexto->ip, 0, sizeof(contexto->ip));
		snprintf(contexto->ip, sizeof(contexto->ip), "%s", ip_cliente);

		if (pthread_create(&tid, NULL, manejar_cliente, contexto) != 0) {
			logger_error("Error creando hilo");
			CLOSE_SOCKET(client_fd);
			free(contexto);
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
