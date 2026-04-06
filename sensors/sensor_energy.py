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
        if random.random() < 0.20: # 20% - WARNING (> 500)
            value = random.uniform(505.0, 900.0)
        else: # 80% - normal (< 500)
            value = random.uniform(150.0, 450.0)
        return round(max(0.0, min(10000.0, value)), 2)
