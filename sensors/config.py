# config.py - Configuración de los sensores simulados
#
# IMPORTANTE: usar hostname, nunca IP hardcodeada.
# El hostname se resuelve mediante DNS en el momento de la conexión.

SERVER_HOST = "iot-monitoring.example.com"  # Cambiar al dominio Route 53 real
SERVER_PORT = 8080

# Intervalos de envío de mediciones (segundos)
SEND_INTERVAL = {
    "temperature": 5,
    "vibration":   3,
    "energy":      7,
}

# Rangos de valores simulados por tipo de sensor
SENSOR_RANGES = {
    "temperature": {"min": -20.0, "max": 120.0, "unit": "celsius"},
    "vibration":   {"min": 0.0,   "max": 15.0,  "unit": "m/s2"},
    "energy":      {"min": 0.0,   "max": 800.0, "unit": "watts"},
}
