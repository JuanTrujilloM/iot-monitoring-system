import socket
import time
from abc import ABC, abstractmethod


class SensorBase(ABC):
    """
    Base class for all IoT sensors.

    Handles TCP connection, registration, measurement sending,
    automatic reconnection on network errors, and console logging.
    Subclasses must implement generate_value().
    """

    def __init__(self, host, port, sensor_id, sensor_type, interval, retry_interval):
        self.host           = host
        self.port           = port
        self.sensor_id      = sensor_id
        self.sensor_type    = sensor_type
        self.interval       = interval
        self.retry_interval = retry_interval
        self._sock          = None

    @abstractmethod
    def generate_value(self):
        """Return the next sensor reading as a float."""

    def _connect(self):
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._sock.settimeout(10)
        self._sock.connect((self.host, self.port))
        self._sock.settimeout(None)
        print(f"[{self.sensor_id}] Connected to {self.host}:{self.port}")

    def _disconnect(self):
        if self._sock:
            try:
                self._sock.close()
            except OSError:
                pass
            self._sock = None

    def _send(self, message: str):
        self._sock.sendall(message.encode())

    def _recv(self) -> str:
        data = self._sock.recv(1024)
        if not data:
            raise ConnectionError("Server closed the connection")
        return data.decode().strip()

    def _register(self):
        self._send(f"REGISTER_SENSOR {self.sensor_type} {self.sensor_id}\r\n")
        response = self._recv()
        if not response.startswith("OK"):
            raise ConnectionError(f"Registration rejected: {response}")
        print(f"[{self.sensor_id}] Registered as type '{self.sensor_type}'")

    def _send_measurement(self, value: float):
        unit = "U"
        if self.sensor_type == "temperature":
            unit = "C"
        elif self.sensor_type == "energy":
            unit = "W"
        elif self.sensor_type == "vibration":
            unit = "mm/s"
            
        self._send(f"MEASUREMENT {self.sensor_id} {value:.4f} {unit}\r\n")
        response = self._recv()
        if not response.startswith("OK"):
            print(f"[{self.sensor_id}] WARNING: unexpected response: {response}")

    def run(self):
        """Start the sensor loop. Blocks until KeyboardInterrupt."""
        while True:
            try:
                self._connect()
                self._register()

                while True:
                    value = self.generate_value()
                    self._send_measurement(value)
                    print(f"[{self.sensor_id}] {self.sensor_type} = {value:.2f}")
                    time.sleep(self.interval)

            except KeyboardInterrupt:
                print(f"[{self.sensor_id}] Interrupted. Disconnecting.")
                self._disconnect()
                break

            except (OSError, ConnectionError) as exc:
                print(f"[{self.sensor_id}] Network error: {exc}")
                self._disconnect()
                print(f"[{self.sensor_id}] Retrying in {self.retry_interval}s ...")
                try:
                    time.sleep(self.retry_interval)
                except KeyboardInterrupt:
                    print(f"[{self.sensor_id}] Interrupted during retry wait. Exiting.")
                    break
