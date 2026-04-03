"""
run_sensors.py — IoT sensor simulator launcher

Usage (single sensor):
  python run_sensors.py --host HOST --port PORT --id SENSOR_ID --type TYPE
                        [--interval SECS] [--retry-interval SECS]

Usage (multiple simultaneous sensors):
  python run_sensors.py --host HOST --port PORT --id SENSOR_ID --type TYPE
                        --count N [--interval SECS] [--retry-interval SECS]

  When --count N > 1, each instance uses ID "<SENSOR_ID>-<1..N>".

Sensor types: temperature | energy | vibration
"""

import argparse
import sys
import threading

from config import DEFAULT_HOST, DEFAULT_PORT, DEFAULT_INTERVAL, DEFAULT_RETRY_INTERVAL
from sensor_temperature import TemperatureSensor
from sensor_energy import EnergySensor
from sensor_vibration import VibrationSensor

SENSOR_CLASSES = {
    "temperature": TemperatureSensor,
    "energy":      EnergySensor,
    "vibration":   VibrationSensor,
}


def build_parser():
    parser = argparse.ArgumentParser(
        description="IoT sensor simulator",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    parser.add_argument("--host",           default=DEFAULT_HOST,
                        help=f"Server host (default: {DEFAULT_HOST})")
    parser.add_argument("--port",           type=int, default=DEFAULT_PORT,
                        help=f"Server TCP port (default: {DEFAULT_PORT})")
    parser.add_argument("--id",             required=True, dest="sensor_id",
                        help="Unique sensor ID, e.g. sensor-temp-001")
    parser.add_argument("--type",           required=True, dest="sensor_type",
                        choices=SENSOR_CLASSES.keys(),
                        help="Sensor type: temperature | energy | vibration")
    parser.add_argument("--interval",       type=float, default=DEFAULT_INTERVAL,
                        help=f"Seconds between measurements (default: {DEFAULT_INTERVAL})")
    parser.add_argument("--retry-interval", type=float, default=DEFAULT_RETRY_INTERVAL,
                        dest="retry_interval",
                        help=f"Seconds before reconnection attempt (default: {DEFAULT_RETRY_INTERVAL})")
    parser.add_argument("--count",          type=int, default=1,
                        help="Number of simultaneous sensor instances (default: 1)")
    return parser


def validate_args(args, parser):
    if args.port < 1 or args.port > 65535:
        parser.error(f"--port must be between 1 and 65535, got {args.port}")
    if args.interval <= 0:
        parser.error(f"--interval must be > 0, got {args.interval}")
    if args.retry_interval <= 0:
        parser.error(f"--retry-interval must be > 0, got {args.retry_interval}")
    if args.count < 1:
        parser.error(f"--count must be >= 1, got {args.count}")


def make_sensor(args, sensor_id):
    cls = SENSOR_CLASSES[args.sensor_type]
    return cls(
        host           = args.host,
        port           = args.port,
        sensor_id      = sensor_id,
        interval       = args.interval,
        retry_interval = args.retry_interval,
    )


def main():
    parser = build_parser()
    args   = parser.parse_args()
    validate_args(args, parser)

    if args.count == 1:
        sensor = make_sensor(args, args.sensor_id)
        sensor.run()
    else:
        threads = []
        for i in range(1, args.count + 1):
            sensor_id = f"{args.sensor_id}-{i}"
            sensor    = make_sensor(args, sensor_id)
            t = threading.Thread(target=sensor.run, name=sensor_id, daemon=True)
            threads.append(t)

        print(f"Starting {args.count} '{args.sensor_type}' sensors ...")
        for t in threads:
            t.start()

        try:
            for t in threads:
                t.join()
        except KeyboardInterrupt:
            print("\nInterrupted. All sensors will stop after the current retry wait.")
            sys.exit(0)


if __name__ == "__main__":
    main()
