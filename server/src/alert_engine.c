/*
 * alert_engine.c - Detección de eventos anómalos y notificación a operadores
 *
 * Responsabilidades:
 *   - Evaluar cada medición contra umbrales predefinidos por tipo de sensor
 *   - Generar alertas cuando se supera un umbral
 *   - Hacer broadcast de alertas a todos los operadores conectados
 *
 * Umbrales sugeridos (ajustar según necesidad):
 *   Temperatura : > 80°C  → HIGH_TEMP  | < -10°C → LOW_TEMP
 *   Vibración   : > 9.0   → HIGH_VIB
 *   Energía     : > 500W  → HIGH_POWER | < 5W    → LOW_POWER
 */

#include "../include/alert_engine.h"
#include "../include/sensor_manager.h"

/* TODO: implementar alert_evaluate(const char *sensor_id, const char *type, double value) */
/* TODO: implementar alert_broadcast(const char *alert_msg)  */
/* TODO: implementar alert_register_operator(int fd)         */
/* TODO: implementar alert_unregister_operator(int fd)       */
