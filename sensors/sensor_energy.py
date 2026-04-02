"""
sensor_energy.py - Sensor de consumo energético

Genera valores simulados de consumo en Watts.
Rango normal: 50W – 400W
Rango anómalo (dispara alerta en servidor): > 500W o < 5W
"""

import random
from sensor_base import SensorBase
from config import SEND_INTERVAL, SENSOR_RANGES


class EnergySensor(SensorBase):
    def __init__(self, sensor_id: str):
        super().__init__(sensor_id, "energy", SEND_INTERVAL["energy"])

    def generate_measurement(self) -> tuple:
        # TODO: generar valor aleatorio con variaciones graduales (simular carga real)
        cfg = SENSOR_RANGES["energy"]
        value = round(random.uniform(cfg["min"], cfg["max"]), 1)
        return value, cfg["unit"]


if __name__ == "__main__":
    sensor = EnergySensor("energy-01")
    sensor.run()
