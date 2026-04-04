#include "alert_engine.h"
#include "logger.h"
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>

static AlertThreshold_t thresholds[MAX_ALERTS];
static ActiveAlert_t active_alerts[MAX_ALERTS];
static int threshold_count = 0;
static int active_alert_count = 0;
static pthread_mutex_t alert_mutex = PTHREAD_MUTEX_INITIALIZER;

int alert_engine_init(void)
{
    pthread_mutex_lock(&alert_mutex);
    memset(thresholds, 0, sizeof(thresholds));
    memset(active_alerts, 0, sizeof(active_alerts));
    threshold_count = 0;
    active_alert_count = 0;
    pthread_mutex_unlock(&alert_mutex);
    logger_info("Alert engine initialized");
    return 0;
}

int alert_engine_register_threshold(const char* sensor_type, double threshold, AlertLevel level, const char* condition, const char* message)
{
    if (threshold_count >= MAX_ALERTS) {
        logger_error("Maximum number of alert thresholds reached");
        return -1;
    }
    if (!sensor_type || strlen(sensor_type) == 0) {
        logger_error("Invalid sensor_type in register_threshold");
        return -1;
    }

    pthread_mutex_lock(&alert_mutex);
    thresholds[threshold_count].sensor_type[15] = '\0';

    strncpy(thresholds[threshold_count].sensor_type, sensor_type, sizeof(thresholds[0].sensor_type) - 1);
    strncpy(thresholds[threshold_count].message, message, sizeof(thresholds[0].message) - 1);
    thresholds[threshold_count].threshold_value = threshold;
    thresholds[threshold_count].level = level;
    strncpy(thresholds[threshold_count].condition, condition, sizeof(thresholds[0].condition) - 1);
    thresholds[threshold_count].is_active = 1;
    thresholds[threshold_count].created_at = time(NULL);
    thresholds[threshold_count].last_triggered = 0;

    char log_msg[128];
    snprintf(log_msg, sizeof(log_msg), "Alert threshold registered for %s: %s %.2f", 
             sensor_type, condition, threshold);
    logger_info(log_msg);

    threshold_count++;

    pthread_mutex_unlock(&alert_mutex);

    return 0;
}

static int check_condition(double value, double threshold, const char* condition)
{
    if (strcmp(condition, ">") == 0) return value > threshold;
    if (strcmp(condition, "<") == 0) return value < threshold;
    if (strcmp(condition, ">=") == 0) return value >= threshold;
    if (strcmp(condition, "<=") == 0) return value <= threshold;
    return 0;
}


static void add_active_alert(const char* sensor_id, const char* sensor_type, double value, AlertLevel level, const char* message)
{
    if (active_alert_count >= MAX_ALERTS) {
        logger_error("Maximum number of active alerts reached");
        return;
    }

    int slot = -1;
    for (int i = 0; i < MAX_ALERTS; i++) {
        if (active_alerts[i].sensor_id[0] == '\0' || active_alerts[i].is_resolved) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        logger_error("No available slot for new alert");
        return;
    }

    strncpy(active_alerts[slot].sensor_id, sensor_id, sizeof(active_alerts[0].sensor_id) - 1);
    strncpy(active_alerts[slot].sensor_type, sensor_type, sizeof(active_alerts[0].sensor_type) - 1);
    active_alerts[slot].value = value;
    active_alerts[slot].level = level;
    active_alerts[slot].timestamp = time(NULL);
    strncpy(active_alerts[slot].message, message, sizeof(active_alerts[0].message) - 1);
    active_alerts[slot].is_resolved = 0;

    if (slot >= active_alert_count) {
        active_alert_count = slot + 1;
    }

    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "ALERT TRIGGERED: %s - %s (%.2f)",
             sensor_id, message, value);
    logger_error(log_msg);
}

int alert_engine_check_measurement(const char* sensor_id, const char* sensor_type, double value)
{
    if (!sensor_id || !sensor_type) return -1;

    int alerts_triggered = 0;

    pthread_mutex_lock(&alert_mutex);

    for (int i = 0; i < threshold_count; i++) {
        if (thresholds[i].is_active && 
            strncmp(thresholds[i].sensor_type, sensor_type, 15) == 0) {

            if (check_condition(value, thresholds[i].threshold_value, thresholds[i].condition)) {

                char alert_msg[MAX_ALERT_MESSAGE];
                snprintf(alert_msg, sizeof(alert_msg), "%s: %.2f %s %.2f",
                         sensor_type, value, thresholds[i].condition, thresholds[i].threshold_value);

                int updated = 0;
                for (int j = 0; j < active_alert_count; j++) {
                    if (strcmp(active_alerts[j].sensor_id, sensor_id) == 0 &&
                        active_alerts[j].level == thresholds[i].level &&
                        !active_alerts[j].is_resolved) {
                        active_alerts[j].value = value;
                        active_alerts[j].timestamp = time(NULL);
                        strncpy(active_alerts[j].message, alert_msg, sizeof(active_alerts[0].message) - 1);
                        updated = 1;
                        break;
                    }
                }

                if (!updated) {
                    add_active_alert(sensor_id, sensor_type, value, thresholds[i].level, alert_msg);
                    alerts_triggered++;
                }

                thresholds[i].last_triggered = time(NULL);
            }
        }
    }

    pthread_mutex_unlock(&alert_mutex);

    return alerts_triggered;
}

int alert_engine_get_active_alerts(char* output_buffer, int buffer_size)
{
    if (!output_buffer || buffer_size <= 0) return -1;

    pthread_mutex_lock(&alert_mutex);

    output_buffer[0] = '\0';
    char temp[512];

    for (int i = 0; i < active_alert_count; i++) {
        if (!active_alerts[i].is_resolved && active_alerts[i].sensor_id[0] != '\0') {
            const char* level_str = (active_alerts[i].level == ALERT_LEVEL_WARNING) ? "WARNING" : "CRITICAL";
            snprintf(temp, sizeof(temp), "%s,%s,%.2f,%s,%ld,%s;",
                     active_alerts[i].sensor_id,
                     active_alerts[i].sensor_type,
                     active_alerts[i].value,
                     level_str,
                     (long)active_alerts[i].timestamp,
                     active_alerts[i].message);

            if (strlen(output_buffer) + strlen(temp) < (size_t)buffer_size) {
                strcat(output_buffer, temp);
            }
        }
    }

    pthread_mutex_unlock(&alert_mutex);
    return 0;
}

int alert_engine_resolve_alert(const char* sensor_id)
{
    if (!sensor_id) return -1;

    int resolved_count = 0;

    pthread_mutex_lock(&alert_mutex);

    for (int i = 0; i < active_alert_count; i++) {
        if (strcmp(active_alerts[i].sensor_id, sensor_id) == 0 && !active_alerts[i].is_resolved) {
            active_alerts[i].is_resolved = 1;
            resolved_count++;

            char log_msg[128];
            snprintf(log_msg, sizeof(log_msg), "Alert resolved for sensor: %s", sensor_id);
            logger_info(log_msg);
        }
    }

    pthread_mutex_unlock(&alert_mutex);
    return resolved_count;
}

void alert_engine_cleanup(void)
{
    pthread_mutex_lock(&alert_mutex);
    threshold_count = 0;
    active_alert_count = 0;
    memset(thresholds, 0, sizeof(thresholds));
    memset(active_alerts, 0, sizeof(active_alerts));
    pthread_mutex_unlock(&alert_mutex);
    logger_info("Alert engine cleaned up");
}
