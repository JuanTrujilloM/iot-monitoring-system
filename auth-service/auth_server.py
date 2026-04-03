import socket
import sqlite3

HOST = '0.0.0.0'
PORT = 9000

def verify_user(username, password):
    conn = sqlite3.connect('users.db')
    c = conn.cursor()
    c.execute("SELECT role FROM users WHERE username=? AND password=?", (username, password))
    result = c.fetchone()
    conn.close()
    return result[0] if result else None

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    print("Servicio de Identidad con base de datos externa ejecutandose en puerto 9000")
    while True:
        conn, addr = s.accept()
        with conn:
            data = conn.recv(1024)
            if not data:
                continue
            decoded = data.decode('utf-8').strip()
            if ":" in decoded:
                username, password = decoded.split(":", 1)
                role = verify_user(username, password)
                if role:
                    conn.sendall(f"ROLE:{role}\n".encode('utf-8'))
                else:
                    conn.sendall(b"ERROR:INVALID\n")
            else:
                conn.sendall(b"ERROR:MALFORMED\n")