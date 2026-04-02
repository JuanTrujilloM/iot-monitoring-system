/*
 * sensor_manager.c - Registro y estado de sensores conectados
 *
 * Responsabilidades:
 *   - Mantener lista de sensores activos (id, tipo, última medición, timestamp)
 *   - Registrar nuevos sensores (REGISTER SENSOR)
 *   - Almacenar mediciones recientes por sensor
 *   - Detectar sensores desconectados (timeout)
 *   - Proveer consultas: lista de activos, mediciones recientes
 */

#include "../include/sensor_manager.h"

/* TODO: implementar sensor_register(const char *id, const char *type)       */
/* TODO: implementar sensor_update(const char *id, double value, const char *unit) */
/* TODO: implementar sensor_list(SensorInfo *out, int max)                   */
/* TODO: implementar sensor_get_measurements(const char *id, int n, Measurement *out) */
/* TODO: implementar sensor_remove(const char *id)                           */
