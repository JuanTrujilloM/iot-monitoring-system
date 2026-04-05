#ifndef ALERT_ENGINE_H
#define ALERT_ENGINE_H

#include <time.h>

#define MAX_ALERTS 100
#define MAX_ALERT_MESSAGE 256

typedef enum {
    ALERT_LEVEL_WARNING = 1,
    ALERT_LEVEL_CRITICAL = 2
} AlertLevel;

typedef struct {
    char sensor_id[32];
    char sensor_type[16];
    double threshold_value;
    AlertLevel level;
    char condition[8];        
    int is_active;
    time_t created_at;
    time_t last_triggered;
    char message[MAX_ALERT_MESSAGE];
} AlertThreshold_t;

typedef struct {
    char sensor_id[32];
    char sensor_type[16];
    double value;
    AlertLevel level;
    time_t timestamp;
    char message[MAX_ALERT_MESSAGE];
    int is_resolved;
} ActiveAlert_t;

int alert_engine_init(void);
int alert_engine_register_threshold(const char* sensor_type, double threshold, AlertLevel level, const char* condition, const char* message);
int alert_engine_check_measurement(const char* sensor_id, const char* sensor_type, double value);
int alert_engine_get_active_alerts(char* output_buffer, int buffer_size);
int alert_engine_resolve_alert(const char* sensor_id);
void alert_engine_cleanup(void);

#endif
