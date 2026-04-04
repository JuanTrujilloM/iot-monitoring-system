import random
from sensor_base import SensorBase


class TemperatureSensor(SensorBase):
    """
    Simulates a temperature sensor.
    Normal range: ~15-35 °C (Gaussian distribution).
    Valid server range: -50 to 150 °C.
    """

    SENSOR_TYPE = "temperature"

    def __init__(self, host, port, sensor_id, interval, retry_interval):
        super().__init__(host, port, sensor_id, self.SENSOR_TYPE, interval, retry_interval)

    def generate_value(self):
        r = random.random()
        if r < 0.05:                          # 5% → CRITICAL (> 50)
            value = random.uniform(51.0, 65.0)
        elif r < 0.25:                        # 20% → WARNING (30 < x <= 50)
            value = random.uniform(31.0, 45.0)
        else:                                 # 75% → normal (< 30)
            value = random.gauss(22.0, 4.0)
        return round(max(-50.0, min(150.0, value)), 2)
