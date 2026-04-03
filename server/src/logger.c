#include "logger.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static FILE *g_log_file = NULL;
static pthread_mutex_t g_logger_mutex = PTHREAD_MUTEX_INITIALIZER;

static unsigned long logger_thread_id(void) {
	return (unsigned long)pthread_self();
}

static void logger_write(const char *level, const char *message) {
	time_t now = time(NULL);
	struct tm local_time;
	char timestamp[32];
	unsigned long tid = logger_thread_id();

	memset(&local_time, 0, sizeof(local_time));

#ifdef _WIN32
	if (localtime_s(&local_time, &now) != 0) {
		memset(&local_time, 0, sizeof(local_time));
	}
#else
	if (localtime_r(&now, &local_time) == NULL) {
		memset(&local_time, 0, sizeof(local_time));
	}
#endif

	if (strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &local_time) == 0) {
		snprintf(timestamp, sizeof(timestamp), "0000-00-00 00:00:00");
	}

	pthread_mutex_lock(&g_logger_mutex);
	fprintf(stdout, "[%s] [%s][Thread %lu] %s\n", timestamp, level, tid, message);
	fflush(stdout);

	if (g_log_file != NULL) {
		fprintf(g_log_file, "[%s] [%s][Thread %lu] %s\n", timestamp, level, tid, message);
		fflush(g_log_file);
	}
	pthread_mutex_unlock(&g_logger_mutex);
}

static void logger_write_event(const char *level, const char *ip, int port, const char *message, const char *response) {
	time_t now = time(NULL);
	struct tm local_time;
	char timestamp[32];
	unsigned long tid = logger_thread_id();
	char line[1536];

	memset(&local_time, 0, sizeof(local_time));

#ifdef _WIN32
	if (localtime_s(&local_time, &now) != 0) {
		memset(&local_time, 0, sizeof(local_time));
	}
#else
	if (localtime_r(&now, &local_time) == NULL) {
		memset(&local_time, 0, sizeof(local_time));
	}
#endif

	if (strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &local_time) == 0) {
		snprintf(timestamp, sizeof(timestamp), "0000-00-00 00:00:00");
	}

	snprintf(
		line,
		sizeof(line),
		"IP=%s Port=%d Message=%s Response=%s",
		ip != NULL ? ip : "unknown",
		port,
		message != NULL ? message : "",
		response != NULL ? response : "");

	pthread_mutex_lock(&g_logger_mutex);
	fprintf(stdout, "[%s] [%s][Thread %lu] %s\n", timestamp, level, tid, line);
	fflush(stdout);

	if (g_log_file != NULL) {
		fprintf(g_log_file, "[%s] [%s][Thread %lu] %s\n", timestamp, level, tid, line);
		fflush(g_log_file);
	}
	pthread_mutex_unlock(&g_logger_mutex);
}

int logger_init(const char *log_path) {
	pthread_mutex_lock(&g_logger_mutex);
	if (g_log_file != NULL) {
		fclose(g_log_file);
		g_log_file = NULL;
	}

	g_log_file = fopen(log_path, "a");
	pthread_mutex_unlock(&g_logger_mutex);

	if (g_log_file == NULL) {
		return -1;
	}

	setvbuf(g_log_file, NULL, _IOLBF, 0);
	return 0;
}

void logger_info(const char *message) {
	logger_write("INFO", message);
}

void logger_error(const char *message) {
	logger_write("ERROR", message);
}

void logger_event(const char *level, const char *ip, int port, const char *message, const char *response) {
	logger_write_event(level, ip, port, message, response);
}

void logger_close(void) {
	pthread_mutex_lock(&g_logger_mutex);
	if (g_log_file != NULL) {
		fflush(g_log_file);
		fclose(g_log_file);
		g_log_file = NULL;
	}
	pthread_mutex_unlock(&g_logger_mutex);
}
