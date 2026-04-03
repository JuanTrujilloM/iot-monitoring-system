#ifndef LOGGER_H
#define LOGGER_H

int logger_init(const char *log_path);
void logger_info(const char *message);
void logger_error(const char *message);
void logger_event(const char *level, const char *ip, int port, const char *message, const char *response);
void logger_close(void);

#endif
