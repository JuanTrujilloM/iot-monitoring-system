#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H
#include <time.h>

#define MAX_SENSORS 50

typedef struct {
    char id[32];
    char type[16];           // "temp", "vib", "energy", etc.
    double last_value;
    char last_unit[16];
    time_t last_timestamp;
    int is_active;
} Sensor_t;

int sensor_manager_init(void);
int sensor_manager_register(const char* sensor_type, const char* sensor_id);
int sensor_manager_add_measurement(const char* sensor_id, double value, const char* unit);
int sensor_manager_get_active_sensors(char* output_buffer, int buffer_size);
int sensor_manager_is_sensor_registered(const char* sensor_id);
void sensor_manager_cleanup(void);

#endif