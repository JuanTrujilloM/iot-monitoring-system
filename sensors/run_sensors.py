"""
run_sensors.py - Lanza 5+ instancias de sensores concurrentes

Crea y arranca en hilos separados:
  - 2 sensores de temperatura (temp-01, temp-02)
  - 2 sensores de vibración   (vib-01,  vib-02)
  - 1 sensor de energía       (energy-01)

Uso:
  python run_sensors.py [--host HOST] [--port PORT]

El host puede sobreescribirse por argumento de consola (HU-10),
pero por defecto usa el valor de config.py (resuelto por DNS).
"""

import threading
import argparse
from sensor_temperature import TemperatureSensor
from sensor_vibration import VibrationSensor
from sensor_energy import EnergySensor


def main():
    parser = argparse.ArgumentParser(description="Simulador de sensores IoT")
    parser.add_argument("--host", help="Hostname del servidor (sobreescribe config.py)")
    parser.add_argument("--port", type=int, help="Puerto del servidor")
    args = parser.parse_args()

    # TODO: si args.host o args.port están presentes, sobreescribir config

    sensors = [
        TemperatureSensor("temp-01"),
        TemperatureSensor("temp-02"),
        VibrationSensor("vib-01"),
        VibrationSensor("vib-02"),
        EnergySensor("energy-01"),
    ]

    threads = []
    for sensor in sensors:
        t = threading.Thread(target=sensor.run, daemon=True)
        t.start()
        threads.append(t)
        print(f"[LAUNCH] Sensor {sensor.sensor_id} ({sensor.sensor_type}) iniciado")

    for t in threads:
        t.join()


if __name__ == "__main__":
    main()
