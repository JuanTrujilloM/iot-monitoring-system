import random
from sensor_base import SensorBase


class EnergySensor(SensorBase):
    """
    Simulates an energy consumption sensor.
    Simulates typical industrial load: 100-500 kWh with occasional peaks.
    Valid server range: 0 to 10 000 kWh.
    """

    SENSOR_TYPE = "energy"

    def __init__(self, host, port, sensor_id, interval, retry_interval):
        super().__init__(host, port, sensor_id, self.SENSOR_TYPE, interval, retry_interval)

    def generate_value(self):
        if random.random() < 0.05:   # 5 % chance of high-load spike
            value = random.uniform(800.0, 1500.0)
        else:
            value = random.uniform(100.0, 500.0)
        return round(max(0.0, min(10000.0, value)), 2)
