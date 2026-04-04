import socket
import time
import re
from abc import ABC, abstractmethod

MAX_MSG_LEN   = 1024
VALID_TYPES   = {"temperature", "energy", "vibration"}
UNIT_MAP      = {"temperature": "C", "energy": "W", "vibration": "mm/s"}
ID_PATTERN    = re.compile(r"^[A-Za-z0-9_-]{1,32}$")

class ProtocolError(Exception):
    """Raised when the server returns an ERROR response."""
    def __init__(self, code: str, description: str):
        self.code = code
        self.description = description
        super().__init__(f"[{code}] {description}")

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
    
    def _parse_response(self, response: str) -> dict:

        parts = response.split(None, 2)

        if not parts:
            return {"status": "UNKNOWN", "code": None, "detail": response}

        status = parts[0].upper()

        if status == "OK":
            detail = parts[1] if len(parts) > 1 else ""
            return {"status": "OK", "code": None, "detail": detail}

        if status == "ERROR":
            code   = parts[1] if len(parts) > 1 else "500"
            desc   = parts[2] if len(parts) > 2 else "Unknown error"
            return {"status": "ERROR", "code": code, "detail": desc}

        return {"status": status, "code": None, "detail": response}

    @staticmethod
    def _validate_measurement(sensor_id: str, value: float, unit: str):
        if not ID_PATTERN.match(sensor_id):
            raise ValueError(f"Invalid sensor_id: '{sensor_id}'")
        if not isinstance(value, (int, float)):
            raise ValueError(f"Value must be numeric, got {type(value)}")
        if not unit:
            raise ValueError("Unit cannot be empty")

    def _register(self):
        self._send(f"REGISTER_SENSOR {self.sensor_id} {self.sensor_type}\r\n")
        response = self._recv()
        if not response.startswith("OK"):
            raise ConnectionError(f"Registration rejected: {response}")
        print(f"[{self.sensor_id}] Registered as type '{self.sensor_type}'")

    def _send_measurement(self, value: float):
        unit = UNIT_MAP.get(self.sensor_type, "U")

        self._validate_measurement(self.sensor_id, value, unit)

        timestamp = int(time.time())

        msg = f"MEASUREMENT {self.sensor_id} {value:.4f} {unit} {timestamp}\r\n"
        self._send(msg)

        response = self._recv()
        parsed   = self._parse_response(response)

        if parsed["status"] == "ERROR":
            print(
                f"[{self.sensor_id}] Server error: "
                f"[{parsed['code']}] {parsed['detail']}"
            )
        elif parsed["status"] != "OK":
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

            except ProtocolError as exc:
                print(f"[{self.sensor_id}] Protocol error: {exc}")
                self._disconnect()
                print(f"[{self.sensor_id}] Retrying in {self.retry_interval}s ...")
                try:
                    time.sleep(self.retry_interval)
                except KeyboardInterrupt:
                    print(f"[{self.sensor_id}] Interrupted during retry. Exiting.")
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
