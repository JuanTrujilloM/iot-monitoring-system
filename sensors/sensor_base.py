"""
sensor_base.py - Clase base para todos los sensores simulados

Responsabilidades:
  - Conectarse al servidor mediante TCP (socket SOCK_STREAM)
  - Resolver el hostname por DNS (socket.getaddrinfo) — nunca IP hardcodeada
  - Enviar REGISTER SENSOR al conectarse
  - Enviar DATA periódicamente según el intervalo configurado
  - Reconectarse automáticamente si la conexión se pierde (HU-08)
  - Manejar excepciones de red sin terminar el proceso
"""

import socket
import time
from config import SERVER_HOST, SERVER_PORT


class SensorBase:
    def __init__(self, sensor_id: str, sensor_type: str, interval: float):
        self.sensor_id = sensor_id
        self.sensor_type = sensor_type
        self.interval = interval
        self.sock = None

    def connect(self):
        """Resuelve el hostname por DNS y establece conexión TCP."""
        # TODO: usar socket.getaddrinfo(SERVER_HOST, SERVER_PORT)
        # TODO: manejar excepción si el DNS falla (continuar sin abortar)
        # TODO: enviar REGISTER SENSOR tras conectar exitosamente
        pass

    def send_message(self, message: str):
        """Envía un mensaje al servidor (terminado en \\n)."""
        # TODO: implementar envío con manejo de errores
        pass

    def read_response(self) -> str:
        """Lee la respuesta del servidor."""
        # TODO: implementar lectura línea a línea
        pass

    def generate_measurement(self) -> tuple:
        """Retorna (valor, unidad). Debe ser sobreescrito por subclases."""
        raise NotImplementedError

    def run(self):
        """Bucle principal: conectar y enviar mediciones periódicamente."""
        while True:
            try:
                self.connect()
                while True:
                    value, unit = self.generate_measurement()
                    # TODO: enviar DATA <sensor_id> <value> <unit>
                    time.sleep(self.interval)
            except Exception as e:
                print(f"[{self.sensor_id}] Error: {e}. Reconectando en 5s...")
                time.sleep(5)
