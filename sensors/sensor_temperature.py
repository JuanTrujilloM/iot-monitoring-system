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
        value = random.gauss(25.0, 5.0)
        return round(max(-50.0, min(150.0, value)), 2)
