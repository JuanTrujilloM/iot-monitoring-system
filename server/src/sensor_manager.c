#include "sensor_manager.h"
#include "logger.h"
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>

static Sensor_t sensors[MAX_SENSORS];
static int sensor_count = 0;
static pthread_mutex_t sensor_mutex = PTHREAD_MUTEX_INITIALIZER;

int sensor_manager_init(void)
{
    pthread_mutex_lock(&sensor_mutex);
    memset(sensors, 0, sizeof(sensors));
    sensor_count = 0;
    pthread_mutex_unlock(&sensor_mutex);
    logger_info("Sensor manager initialized");
    return 0;
}

int sensor_manager_register(const char* sensor_type, const char* sensor_id)
{
    if (!sensor_type || !sensor_id) return -1;

    pthread_mutex_lock(&sensor_mutex);

   
    for (int i = 0; i < sensor_count; i++) {
        if (strcmp(sensors[i].id, sensor_id) == 0) {
            sensors[i].is_active = 1;
            pthread_mutex_unlock(&sensor_mutex);
            logger_event("INFO", "0.0.0.0", 0, "Sensor already registered", sensor_id);
            return 0;
        }
    }

    
    if (sensor_count >= MAX_SENSORS) {
        pthread_mutex_unlock(&sensor_mutex);
        logger_error("Maximum number of sensors reached");
        return -1;
    }

    strncpy(sensors[sensor_count].id, sensor_id, sizeof(sensors[0].id) - 1);
    strncpy(sensors[sensor_count].type, sensor_type, sizeof(sensors[0].type) - 1);
    sensors[sensor_count].last_value = 0.0;
    sensors[sensor_count].last_unit[0] = '\0';
    sensors[sensor_count].last_timestamp = time(NULL);
    sensors[sensor_count].is_active = 1;

    sensor_count++;

    pthread_mutex_unlock(&sensor_mutex);

    char msg[128];
    snprintf(msg, sizeof(msg), "Sensor registered: %s (%s)", sensor_id, sensor_type);
    logger_info(msg);

    return 0;
}

int sensor_manager_add_measurement(const char* sensor_id, double value, const char* unit)
{
    if (!sensor_id || !unit) return -1;

    pthread_mutex_lock(&sensor_mutex);

    for (int i = 0; i < sensor_count; i++) {
        if (strcmp(sensors[i].id, sensor_id) == 0) {
            sensors[i].last_value = value;
            strncpy(sensors[i].last_unit, unit, sizeof(sensors[0].last_unit) - 1);
            sensors[i].last_timestamp = time(NULL);
            sensors[i].is_active = 1;

            pthread_mutex_unlock(&sensor_mutex);

            char log_msg[128];
            snprintf(log_msg, sizeof(log_msg), "Measurement from %s: %.2f %s", sensor_id, value, unit);
            logger_info(log_msg);

            return 0;
        }
    }

    pthread_mutex_unlock(&sensor_mutex);
    logger_error("Measurement received from unregistered sensor");
    return -1;
}

int sensor_manager_get_active_sensors(char* output_buffer, int buffer_size)
{
    if (!output_buffer || buffer_size <= 0) return -1;

    pthread_mutex_lock(&sensor_mutex);

    output_buffer[0] = '\0';
    char temp[128];

    for (int i = 0; i < sensor_count; i++) {
        if (sensors[i].is_active) {
            snprintf(temp, sizeof(temp), "%s,%s,%.2f,%s;",
                     sensors[i].id,
                     sensors[i].type,
                     sensors[i].last_value,
                     sensors[i].last_unit);
            
            if (strlen(output_buffer) + strlen(temp) < (size_t)buffer_size) {
                strcat(output_buffer, temp);
            }
        }
    }

    pthread_mutex_unlock(&sensor_mutex);
    return 0;
}

int sensor_manager_is_sensor_registered(const char* sensor_id)
{
    if (!sensor_id) return 0;

    pthread_mutex_lock(&sensor_mutex);
    for (int i = 0; i < sensor_count; i++) {
        if (strcmp(sensors[i].id, sensor_id) == 0) {
            pthread_mutex_unlock(&sensor_mutex);
            return 1;
        }
    }
    pthread_mutex_unlock(&sensor_mutex);
    return 0;
}

const char* sensor_manager_get_sensor_type(const char* sensor_id)
{
    if (!sensor_id) return NULL;

    pthread_mutex_lock(&sensor_mutex);
    for (int i = 0; i < sensor_count; i++) {
        if (strcmp(sensors[i].id, sensor_id) == 0) {
            pthread_mutex_unlock(&sensor_mutex);
            return sensors[i].type;
        }
    }
    pthread_mutex_unlock(&sensor_mutex);
    return NULL;
}

void sensor_manager_cleanup(void)
{
    pthread_mutex_lock(&sensor_mutex);
    sensor_count = 0;
    pthread_mutex_unlock(&sensor_mutex);
    logger_info("Sensor manager cleaned up");
}