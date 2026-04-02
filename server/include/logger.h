#ifndef LOGGER_H
#define LOGGER_H

int logger_init(const char *ruta_archivo);
void logger_info(const char *mensaje);
void logger_error(const char *mensaje);
void logger_evento(const char *nivel, const char *ip, int puerto, const char *mensaje, const char *respuesta);
void logger_close(void);

#endif
