"""
sensor_vibration.py - Sensor de vibración mecánica

Genera valores simulados de vibración en m/s².
Rango normal: 0.0 – 7.0 m/s²
Rango anómalo (dispara alerta en servidor): > 9.0 m/s²
"""

import random
from sensor_base import SensorBase
from config import SEND_INTERVAL, SENSOR_RANGES


class VibrationSensor(SensorBase):
    def __init__(self, sensor_id: str):
        super().__init__(sensor_id, "vibration", SEND_INTERVAL["vibration"])

    def generate_measurement(self) -> tuple:
        # TODO: generar valor aleatorio con picos ocasionales de vibración alta
        cfg = SENSOR_RANGES["vibration"]
        value = round(random.uniform(cfg["min"], cfg["max"]), 3)
        return value, cfg["unit"]


if __name__ == "__main__":
    sensor = VibrationSensor("vib-01")
    sensor.run()
