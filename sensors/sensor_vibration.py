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
        if random.random() < 0.05:   # 5 % chance of vibration spike
            value = random.uniform(70.0, 100.0)
        else:
            value = random.uniform(0.0, 30.0)
        return round(max(0.0, min(100.0, value)), 2)
