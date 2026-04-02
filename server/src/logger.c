#include "logger.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static FILE *g_archivo_log = NULL;
static pthread_mutex_t g_logger_mutex = PTHREAD_MUTEX_INITIALIZER;

static unsigned long logger_thread_id(void) {
	return (unsigned long)pthread_self();
}

static void logger_escribir(const char *nivel, const char *mensaje) {
	time_t ahora = time(NULL);
	struct tm tiempo_local;
	char timestamp[32];
	unsigned long tid = logger_thread_id();

	memset(&tiempo_local, 0, sizeof(tiempo_local));

#ifdef _WIN32
	if (localtime_s(&tiempo_local, &ahora) != 0) {
		memset(&tiempo_local, 0, sizeof(tiempo_local));
	}
#else
	if (localtime_r(&ahora, &tiempo_local) == NULL) {
		memset(&tiempo_local, 0, sizeof(tiempo_local));
	}
#endif

	if (strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &tiempo_local) == 0) {
		snprintf(timestamp, sizeof(timestamp), "0000-00-00 00:00:00");
	}

	pthread_mutex_lock(&g_logger_mutex);
	fprintf(stdout, "[%s] [%s][Thread %lu] %s\n", timestamp, nivel, tid, mensaje);
	fflush(stdout);

	if (g_archivo_log != NULL) {
		fprintf(g_archivo_log, "[%s] [%s][Thread %lu] %s\n", timestamp, nivel, tid, mensaje);
		fflush(g_archivo_log);
	}
	pthread_mutex_unlock(&g_logger_mutex);
}

static void logger_escribir_evento(const char *nivel, const char *ip, int puerto, const char *mensaje, const char *respuesta) {
	time_t ahora = time(NULL);
	struct tm tiempo_local;
	char timestamp[32];
	unsigned long tid = logger_thread_id();
	char linea[1536];

	memset(&tiempo_local, 0, sizeof(tiempo_local));

#ifdef _WIN32
	if (localtime_s(&tiempo_local, &ahora) != 0) {
		memset(&tiempo_local, 0, sizeof(tiempo_local));
	}
#else
	if (localtime_r(&ahora, &tiempo_local) == NULL) {
		memset(&tiempo_local, 0, sizeof(tiempo_local));
	}
#endif

	if (strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &tiempo_local) == 0) {
		snprintf(timestamp, sizeof(timestamp), "0000-00-00 00:00:00");
	}

	snprintf(
		linea,
		sizeof(linea),
		"IP=%s Puerto=%d Mensaje=%s Respuesta=%s",
		ip != NULL ? ip : "desconocida",
		puerto,
		mensaje != NULL ? mensaje : "",
		respuesta != NULL ? respuesta : "");

	pthread_mutex_lock(&g_logger_mutex);
	fprintf(stdout, "[%s] [%s][Thread %lu] %s\n", timestamp, nivel, tid, linea);
	fflush(stdout);

	if (g_archivo_log != NULL) {
		fprintf(g_archivo_log, "[%s] [%s][Thread %lu] %s\n", timestamp, nivel, tid, linea);
		fflush(g_archivo_log);
	}
	pthread_mutex_unlock(&g_logger_mutex);
}

int logger_init(const char *ruta_archivo) {
	pthread_mutex_lock(&g_logger_mutex);
	if (g_archivo_log != NULL) {
		fclose(g_archivo_log);
		g_archivo_log = NULL;
	}

	g_archivo_log = fopen(ruta_archivo, "a");
	pthread_mutex_unlock(&g_logger_mutex);

	if (g_archivo_log == NULL) {
		return -1;
	}

	setvbuf(g_archivo_log, NULL, _IOLBF, 0);
	return 0;
}

void logger_info(const char *mensaje) {
	logger_escribir("INFO", mensaje);
}

void logger_error(const char *mensaje) {
	logger_escribir("ERROR", mensaje);
}

void logger_evento(const char *nivel, const char *ip, int puerto, const char *mensaje, const char *respuesta) {
	logger_escribir_evento(nivel, ip, puerto, mensaje, respuesta);
}

void logger_close(void) {
	pthread_mutex_lock(&g_logger_mutex);
	if (g_archivo_log != NULL) {
		fflush(g_archivo_log);
		fclose(g_archivo_log);
		g_archivo_log = NULL;
	}
	pthread_mutex_unlock(&g_logger_mutex);
}
