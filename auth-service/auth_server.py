"""
auth_server.py - Servicio externo de autenticación

Escucha en TCP puerto 9000 (configurable).
El servidor C se conecta aquí para validar credenciales.

Protocolo interno (texto, una línea):
  Petición:  AUTH_CHECK <usuario> <password>\n
  Respuesta: AUTH_OK <rol>\n
             AUTH_FAIL <motivo>\n

Usuarios y roles se leen de users.json.
Las contraseñas deben almacenarse hasheadas (nunca en texto plano).

Uso:
  python auth_server.py [--port 9000]
"""

import socket
import json
import hashlib
import argparse

AUTH_PORT = 9000
USERS_FILE = "users.json"


def load_users(filepath: str) -> dict:
    """Carga el archivo de usuarios y retorna un dict {username: {hash, role}}."""
    # TODO: implementar carga y parseo de users.json
    pass


def verify_credentials(users: dict, username: str, password: str):
    """Retorna el rol si las credenciales son válidas, None si no."""
    # TODO: hashear password y comparar con el almacenado
    pass


def handle_client(conn: socket.socket, addr, users: dict):
    """Atiende una petición de verificación."""
    # TODO: leer línea AUTH_CHECK <user> <pass>
    # TODO: llamar verify_credentials
    # TODO: responder AUTH_OK <rol> o AUTH_FAIL INVALID_CREDENTIALS
    pass


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", type=int, default=AUTH_PORT)
    args = parser.parse_args()

    users = load_users(USERS_FILE)

    # TODO: crear socket TCP, bind, listen
    # TODO: bucle accept -> handle_client en hilo separado
    print(f"[AUTH] Servicio de autenticación escuchando en puerto {args.port}")


if __name__ == "__main__":
    main()
