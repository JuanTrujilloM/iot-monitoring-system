#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <time.h>

#define MAX_SENSORS         32
#define MAX_MEASUREMENTS    100
#define SENSOR_ID_LEN       64
#define SENSOR_TYPE_LEN     32
#define SENSOR_UNIT_LEN     16

/* TODO: definir struct Measurement { double value; char unit[]; time_t timestamp; } */
/* TODO: definir struct SensorInfo   { char id[]; char type[]; Measurement last; int active; } */
/* TODO: declarar sensor_register, sensor_update, sensor_list,
         sensor_get_measurements, sensor_remove                                         */

#endif /* SENSOR_MANAGER_H */
