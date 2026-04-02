"""
sensor_temperature.py - Sensor de temperatura

Genera valores simulados de temperatura en grados Celsius.
Rango normal: 15°C – 75°C
Rango anómalo (dispara alerta en servidor): > 80°C o < -10°C
"""

import random
from sensor_base import SensorBase
from config import SEND_INTERVAL, SENSOR_RANGES


class TemperatureSensor(SensorBase):
    def __init__(self, sensor_id: str):
        super().__init__(sensor_id, "temperature", SEND_INTERVAL["temperature"])

    def generate_measurement(self) -> tuple:
        # TODO: generar valor aleatorio con distribución realista
        # Sugerencia: usar random.gauss() centrado en 40°C, con picos ocasionales
        cfg = SENSOR_RANGES["temperature"]
        value = round(random.uniform(cfg["min"], cfg["max"]), 2)
        return value, cfg["unit"]


if __name__ == "__main__":
    sensor = TemperatureSensor("temp-01")
    sensor.run()
