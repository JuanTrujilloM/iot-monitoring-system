import random
from sensor_base import SensorBase


class VibrationSensor(SensorBase):
    """
    Simulates a vibration sensor.
    Normal range: 0-30 (level). Occasional spikes to 70-100.
    Valid server range: 0 to 100.
    """

    SENSOR_TYPE = "vibration"

    def __init__(self, host, port, sensor_id, interval, retry_interval):
        super().__init__(host, port, sensor_id, self.SENSOR_TYPE, interval, retry_interval)

    def generate_value(self):
        r = random.random()
        if r < 0.10: # 10% - CRITICAL (> 10)
            value = random.uniform(11.0, 20.0)
        elif r < 0.30: # 20% - WARNING (5 < x <= 10)
            value = random.uniform(5.5, 9.5)
        else: # 70% - normal (< 5)
            value = random.uniform(0.5, 4.5)
        return round(max(0.0, min(100.0, value)), 2)
