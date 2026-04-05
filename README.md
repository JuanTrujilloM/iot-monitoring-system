# IoT Distributed Sensor Monitoring System

## Project Context

This project implements a distributed IoT monitoring system for industrial sensor data collection and real-time alerting. The system simulates a factory environment where multiple sensors (temperature, energy consumption, and vibration) continuously report measurements to a central server. Operators can monitor the system through a web dashboard that displays sensor data and alerts in real-time.

The system is designed to demonstrate distributed systems concepts, network programming, and real-time data processing in a practical IoT scenario. It includes components written in multiple languages (C, Python, Java) to showcase interoperability between different technologies.

## Applied Theoretical Concepts

### Distributed Systems
- **Client-Server Architecture**: Multiple sensor clients connect to a central C server via TCP
- **Service-Oriented Architecture**: Separate authentication service (Python) and monitoring server (C)
- **Real-time Communication**: WebSocket integration for live dashboard updates

### Network Programming
- **TCP/IP Sockets**: Reliable communication between all components
- **Text-based Protocols**: Human-readable message formats for simplicity and debugging
- **Connection Management**: Persistent connections with authentication and health checks

### Concurrent Programming
- **Multi-threading**: Server handles multiple simultaneous sensor connections
- **Asynchronous Operations**: Non-blocking I/O for sensor data processing
- **Thread Safety**: Proper synchronization for shared data structures

### Data Processing & Alerting
- **Threshold-based Monitoring**: Configurable alert rules for sensor measurements
- **Real-time Analytics**: Immediate processing of incoming sensor data
- **Event-driven Architecture**: Alert generation triggers notifications to operators

### Security Concepts
- **Authentication**: User credential validation via dedicated auth service
- **Authorization**: Role-based access control (operator/admin)
- **Input Validation**: Protocol-level validation to prevent malformed data

## Project Architecture

The system consists of four main components:

```
┌─────────────────┐    ┌─────────────────┐
│   Auth Service  │    │   C Server      │
│   (Python)      │    │   (Core)        │
│   Port: 9000    │    │   Port: 8080    │
│                 │    │                 │
│ - User auth     │    │ - Sensor mgr    │
│ - SQLite DB     │    │ - Alert engine  │
└─────────┬───────┘    └───────┬─────────┘
          │                    │
          │                    │
          ▼                    ▼
┌─────────────────┐    ┌─────────────────┐
│  Sensor Nodes   │    │ Operator Client │
│   (Python)      │    │   (Java/Spring) │
│                 │    │   Port: 8081    │
│ - Temp sensors  │    │                 │
│ - Energy sensors│    │ - Web dashboard │
│ - Vibration     │    │ - WebSocket     │
└─────────────────┘    └─────────────────┘
```

### Component Details

**Auth Service (Python)**
- TCP server on port 9000
- Validates user credentials against SQLite database
- Returns user roles for authorization

**C Server (Core Monitoring)**
- TCP server on port 8080
- Manages sensor registrations and data collection
- Processes measurements against alert thresholds
- Broadcasts alerts to connected operator clients

**Sensor Nodes (Python)**
- Simulate industrial sensors (temperature, energy, vibration)
- Connect to C server via TCP
- Send periodic measurements using text protocol
- Support multiple simultaneous instances

**Operator Client (Java/Spring Boot)**
- Web dashboard on port 8081
- Real-time sensor monitoring via WebSocket
- Alert notifications and historical data display
- Connects to C server for data retrieval

## Protocol Description

The system uses a simple text-based TCP protocol for all inter-component communication. Messages are line-terminated commands with space-separated arguments, designed for human readability and easy debugging.

For detailed protocol specifications including message formats, command types, authentication flows, and examples, see [PROTOCOL.md](PROTOCOL.md).

## How It Works

### System Startup
1. **Auth Service**: Starts on port 9000, loads user database
2. **C Server**: Starts on port 8080, initializes sensor manager and alert engine with predefined thresholds
3. **Operator Client**: Web application starts on port 8081
4. **Sensor Nodes**: Multiple sensor simulators start and connect to the C server

### Normal Operation Flow
1. **Sensor Registration**: Each sensor authenticates and registers with the server
2. **Data Collection**: Sensors send periodic measurements (temperature, energy, vibration)
3. **Threshold Monitoring**: Server checks each measurement against alert thresholds
4. **Alert Generation**: When thresholds exceeded, alerts are created and broadcast
5. **Real-time Updates**: Operator dashboard receives live sensor data and alerts via WebSocket

### Authentication Flow
- Sensors and operators authenticate via the Auth Service
- Credentials validated against database
- Roles returned for access control
- Server maintains authenticated connection registry

### Alert System
- Configured thresholds at server startup (e.g., temperature > 30°C = warning)
- Measurements continuously evaluated against rules
- Alerts broadcast immediately to all connected operator clients
- Dashboard displays alerts with severity levels and timestamps

## Installation & Setup

### Prerequisites
- Docker and Docker Compose
- Java 17+ (for operator client)
- Python 3.8+ (for sensors and auth service)
- GCC compiler (for C server)

### Quick Start with Docker
```bash
# Build and start all services
docker-compose up --build

# Access dashboard at http://localhost:8081
```

### Manual Setup

#### 1. Auth Service
```bash
cd auth-service
python auth_server.py
```

#### 2. C Server
```bash
cd server
make
./server 8080 server.log
```

#### 3. Operator Client
```bash
cd operator-client
mvn spring-boot:run
```

#### 4. Sensor Nodes
```bash
cd sensors
# Single temperature sensor
python run_sensors.py --host localhost --port 8080 --id sensor-temp-001 --type temperature

# Multiple sensors
python run_sensors.py --host localhost --port 8080 --id sensor --type temperature --count 3
```

## Usage Examples

### Monitoring Dashboard
- Open http://localhost:8081 in browser
- Login with credentials (admin/admin123 or operator/operator123)
- View real-time sensor data and alerts

### Sensor Simulation
```bash
# Temperature sensor with 2-second intervals
python run_sensors.py --host localhost --port 8080 --id temp-sensor --type temperature --interval 2

# Energy sensor with custom retry interval
python run_sensors.py --host localhost --port 8080 --id energy-sensor --type energy --retry-interval 5
```

### Server Logs
```bash
tail -f server/server.log
```

## Configuration

### Server Thresholds
Modify `server/src/main.c` to adjust alert thresholds:
```c
alert_engine_register_threshold("temperature", 30.0, ALERT_LEVEL_WARNING, ">", "Temperature above normal");
```

### Database Setup
Auth service uses SQLite. Initialize with:
```bash
cd auth-service
python init_db.py
```

### Application Properties
Operator client configuration in `operator-client/src/main/resources/application.properties`:
```properties
iot.server.host=localhost
iot.server.port=8080
```

## Development

### Building Components
```bash
# C Server
cd server && make clean && make

# Java Client
cd operator-client && mvn clean compile

# Python components (no build required)
```

### Testing
- Use `PING` command to test server connectivity
- Monitor `server.log` for connection events
- Check WebSocket messages in browser developer tools

## Troubleshooting

### Common Issues
- **Connection refused**: Ensure all services are running on correct ports
- **Authentication failed**: Check credentials in auth service database
- **No sensor data**: Verify sensor registration completed successfully
- **WebSocket errors**: Check firewall settings and port availability

### Logs
- Server logs: `server/server.log`
- Auth service: Console output
- Operator client: Spring Boot logs
- Sensors: Console output with connection status

## Future Enhancements

- Database persistence for sensor data
- TLS/SSL encryption for secure communication
- REST API for external integrations
- Advanced analytics and reporting
- Mobile app for remote monitoring
- Sensor calibration and maintenance tracking