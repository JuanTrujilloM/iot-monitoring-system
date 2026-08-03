#!/bin/sh
set -e

AUTH_PORT="${IOT_AUTH_PORT:-9000}"
SERVER_PORT="${IOT_SERVER_PORT:-8080}"

wait_for_port() {
    name="$1"
    port="$2"
    i=0
    while [ "$i" -lt 30 ]; do
        if python3 -c "import socket,sys; s=socket.socket(); s.settimeout(1); sys.exit(0 if s.connect_ex(('127.0.0.1',$port))==0 else 1)"; then
            echo "[start-demo] $name is up on :$port"
            return 0
        fi
        i=$((i + 1))
        sleep 1
    done
    echo "[start-demo] ERROR: $name never came up on :$port" >&2
    exit 1
}

echo "[start-demo] starting auth-service"
cd /app/auth-service && python3 auth_server.py &
wait_for_port "auth-service" "$AUTH_PORT"

echo "[start-demo] starting central-server"
cd /app && ./central-server "$SERVER_PORT" /app/logs/server.log &
wait_for_port "central-server" "$SERVER_PORT"

# --count runs the instances as threads in one process, so a type costs the
# same whether it has one sensor or several.
echo "[start-demo] starting sensors"
cd /app/sensors
python3 run_sensors.py --id demo-temp --type temperature --count "${SENSORS_PER_TYPE:-5}" &
python3 run_sensors.py --id demo-energy --type energy --count "${SENSORS_PER_TYPE:-5}" &
python3 run_sensors.py --id demo-vib --type vibration --count "${SENSORS_PER_TYPE:-5}" &

echo "[start-demo] starting dashboard on :${PORT:-8090}"
cd /app
exec java -Xmx200m -XX:MaxMetaspaceSize=96m -jar operator-client.jar
