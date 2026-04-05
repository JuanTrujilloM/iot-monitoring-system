# IoT Monitoring System - Protocol Documentation

## Overview

The IoT Monitoring System is a distributed architecture consisting of multiple components that communicate through text-based TCP/IP protocols. The system includes:

- **C Server**: Core monitoring server (TCP port 8080) that manages sensors, stores data, and triggers alerts
- **Auth Service**: Python authentication service (TCP port 9000) that validates user credentials
- **Sensor Nodes**: Python-based remote sensor simulators that connect to the main server
- **Operator Client**: Java/Spring Boot web dashboard that communicates with the C server via WebSocket

All inter-component communication uses line-terminated text protocols (messages end with `\r\n`) designed for simplicity and human readability.

---

## System Architecture

```
┌─────────────────┐
│   Auth Service  │      Validates user credentials
│   (Python)      │
│   Port: 9000    │
└────────┬────────┘
         │
         ▼
    [Socket]
    username:password
    ──────────────────
    ROLE:operator/admin
         
┌──────────────────────────────────────────┐
│         C Server (Core)                  │
│      (TCP Port: 8080)                    │
│                                          │
│  ├─ Sensor Manager                       │
│  ├─ Alert Engine                         │
│  └─ Connected Clients Registry           │
└────▲──────────────────▲──────────────────┘
     │                  │
     │                  │
  [TCP]            [WebSocket]
   Port            Port 8080
   8080            (HTTP Upgrade)
     │                  │
     │                  ▼
┌────┴──────────────────────────┐
│  Sensor Nodes (Python)        │
│  - temperature                │
│  - energy                     │
│  - vibration                  │
│  (Multiple instances)         │
└──────────────────────────────┘

     ┌────────────────────────────┐
     │  Operator Dashboard        │
     │  (Java/Spring Boot)        │
     │  http://localhost:8081     │
     └────────────────────────────┘
```

---

## Main Protocol: TCP Text Messages

The server and clients communicate via TCP using line-terminated commands. Each message is a space-separated command followed by arguments, ending with `\r\n`.

### Message Structure
```
COMMAND [ARG1] [ARG2] ... [ARGN]\r\n
```

### Command Types

| Command | Sender | Purpose | Arguments |
|---------|--------|---------|-----------|
| `LOGIN` | Sensor/Client | Authenticate to the server | `<username> <password>` |
| `OPERATOR_IDENTIFY` | Operator Client | Identify as operator connection | (no args) |
| `REGISTER_SENSOR` | Sensor | Register a new sensor | `<sensor_id> <sensor_type>` |
| `MEASUREMENT` | Sensor | Report sensor reading | `<sensor_id> <sensor_type> <value>` |
| `GET_SENSORS` | Client | Retrieve sensor list | (no args) |
| `GET_STATUS` | Client | Get server status | (no args) |
| `GET_ALERTS` | Client | Get alert list | (no args) |
| `PING` | Any | Health check | (no args) |

---

## Sensor Types

```c
SENSOR_TYPE_TEMPERATURE = 1    // Temperature in Celsius (°C)
SENSOR_TYPE_ENERGY = 2         // Energy consumption (kWh)
SENSOR_TYPE_VIBRATION = 3      // Vibration level (0-100)
```

### Acceptable Value Ranges

| Sensor Type | Min | Max | Unit |
|------------|-----|-----|------|
| Temperature | -50 | 150 | °C |
| Energy | 0 | 10000 | kWh |
| Vibration | 0 | 100 | Level |

---

## Server Responses

The server responds to commands with status messages in the following format:

```
STATUS_CODE:SENSOR_ID:MESSAGE
```

Alternatively, data responses for queries:

```
SENSORS [list_data]
ALERTA sensor_id message
STATUS [status_text]
```

### Common Responses

| Response | Meaning | Example |
|----------|---------|---------|
| `OK [details]` | Command succeeded | `OK Temperature registered` |
| `ERROR [code]` | Command failed | `ERROR INVALID_FORMAT` |
| `SENSORS ...` | Sensor list data | `SENSORS sensor1:temp:25.5 ...` |
| `ALERTA` | Alert notification | `ALERTA sensor1 Temperature above threshold` |
| `PONG` | Response to PING | `PONG` |

---

## Alert System

The alert engine monitors thresholds registered at server startup:

```c
alert_engine_register_threshold("temperature", 30.0, WARNING, ">", "Temperature above normal");
alert_engine_register_threshold("temperature", 50.0, CRITICAL, ">", "Temperature critically high");
alert_engine_register_threshold("vibration", 5.0, WARNING, ">", "Vibration above normal");
alert_engine_register_threshold("vibration", 10.0, CRITICAL, ">", "Vibration critically high");
alert_engine_register_threshold("energy", 500.0, WARNING, ">", "Energy consumption high");
```

When a sensor measurement exceeds a threshold:
1. The alert engine creates an alert
2. The server broadcasts to all connected operator clients:
   ```
   ALERTA <sensor_id> <alert_message>
   ```
3. Operator clients receive updates in real-time via WebSocket

---

## Protocol Flow Examples

### Example 1: Sensor Registration Flow

**Sensor node starts and connects to server (TCP):**
```
Sensor → Server:  LOGIN sensor sensor_password
Server → Sensor:  OK Authenticated
               
Sensor → Server:  REGISTER_SENSOR sensor-temp-001 1
Server → Sensor:  OK sensor-temp-001 registered
```

### Example 2: Sensor Data Submission

```
Sensor → Server:  MEASUREMENT sensor-temp-001 1 22.3
Server → Sensor:  OK sensor-temp-001 Data recorded
```

**Meaning:** Sensor `sensor-temp-001` (type 1 = Temperature) reports 22.3°C. Server acknowledges and stores the reading.

### Example 3: Out of Range Measurement

**Sensor sends:**
```
Sensor → Server:  MEASUREMENT sensor-temp-001 1 200.0
Server → Sensor:  ERROR OUT_OF_RANGE Temperature exceeds maximum
```

**Meaning:** Temperature of 200°C exceeds the maximum threshold (150°C), so the server rejects the value and triggers an alert.

### Example 4: Operator Client Dashboard Connection Flow

**Operator client connects via WebSocket:**
```
Operator → Server:  LOGIN operator operator_password
Server → Operator:  OK Authenticated

Operator → Server:  OPERATOR_IDENTIFY
Server → Operator:  OK WebSocket operator connection established

Operator → Server:  GET_SENSORS
Server → Operator:  SENSORS sensor-temp-001:1:22.3 sensor-energy-001:2:145.6 ...
```

### Example 5: Alert Broadcast

**When a measurement triggers an alert:**
```
Sensor → Server:  MEASUREMENT sensor-temp-001 1 35.5
Server:  [Alert engine detects 35.5 > 30.0 threshold]
Server → All Operators:  ALERTA sensor-temp-001:Temperature above normal
```

---

## Authentication Service (Port 9000)

A separate Python service handles credential validation.

### Authentication Handshake

**Client connects to auth service:**
```
Client → Auth:  username:password
Auth → Client:  ROLE:operator    [if credentials valid]
         OR     ERROR:INVALID    [if credentials invalid]
```

**Example:**
```
Client → Auth:  admin:admin123
Auth → Client:  ROLE:admin
```

---

## WebSocket Integration (Port 8080)

The Operator Client uses WebSocket to receive real-time updates from the server:

- **Real-time sensor updates:** Broadcast to all connected operators every 2 seconds (polling via `GET_SENSORS`)
- **Instant alert notifications:** Pushed immediately when thresholds exceeded
- **Bidirectional communication:** Operators can send commands, server responds with data

### WebSocket Message Format

Alert messages are broadcast to WebSocket topic `/topic/alerts`:
```json
{
  "message": "Temperature above normal",
  "sensorId": "sensor-temp-001",
  "severity": "WARNING",
  "timestamp": "2026-04-04T10:30:45Z"
}
```

---

## Server Implementation

The C server includes:

- **Port 8080:** TCP listener for sensor + operator connections
- **Sensor Manager:** Tracks registered sensors and their latest readings
- **Alert Engine:** Monitors measurements against configured thresholds
- **Logging:** All events written to `server.log`

### Key Thresholds (at Startup)

```c
Temperature: WARNING >= 30°C, CRITICAL >= 50°C
Vibration:   WARNING >= 5.0, CRITICAL >= 10.0
Energy:      WARNING >= 500 kWh
```

---

## Error Handling

The server gracefully handles:
- **Malformed messages:** Returns `ERROR MALFORMED_COMMAND`
- **Unknown sensors:** Returns `ERROR INVALID_SENSOR` (sensor not registered)
- **Out-of-range values:** Returns `ERROR OUT_OF_RANGE` + triggers alert
- **Network timeouts:** Auto-reconnection with configurable retry interval
- **Authentication failures:** Connection rejected with `ERROR INVALID`

---

## Configuration

### Server Start
```bash
./server 8080 server.log
```

### Sensor Start
```bash
python run_sensors.py --host localhost --port 8080 \
  --id sensor-temp-001 --type temperature --interval 2
```

### Multiple Sensors (Simultaneous)
```bash
python run_sensors.py --host localhost --port 8080 \
  --id sensor-temp --type temperature --count 3 --interval 2
# Spawns: sensor-temp-1, sensor-temp-2, sensor-temp-3
```

---

## Best Practices

1. **Keep-Alive:** Send `PING` periodically to detect dead connections
2. **Reconnection:** Sensors automatically retry on connection loss (configurable `--retry-interval`)
3. **Measurement Frequency:** Balance between data freshness and server load (default: 2 seconds)
4. **Logging:** Enable `server.log` for debugging connection issues
5. **Credential Security:** Change default passwords in production

---

## Future Enhancements

- Add TLS/SSL encryption for secure communication
- Support batch sensor measurements in single message
- Add sensor data persistence (database integration)
- Implement persistent alert history
- Add sensor self-diagnostics and health checks
