# IoT Monitoring System - Protocol Documentation

## Overview

The IoT Monitoring System uses a simple text-based protocol for communication between sensors, the server, and clients. The protocol is designed to be lightweight, human-readable, and easy to parse.

## Protocol Message Format

### Sensor Data Message (Type 1)
Sensors send data to the server with this format:

```
TYPE:SENSOR_ID:SENSOR_TYPE:VALUE:TIMESTAMP
```

**Example:**
```
1:42:1:25.5:1712177000
```

| Field | Type | Description | Example |
|-------|------|-------------|---------|
| TYPE | int | Message type (1=SENSOR_DATA) | `1` |
| SENSOR_ID | int | Unique sensor identifier | `42` |
| SENSOR_TYPE | int | Type of sensor (1=TEMP, 2=ENERGY, 3=VIBRATION) | `1` |
| VALUE | float | Sensor reading value | `25.5` |
| TIMESTAMP | long | Unix timestamp of measurement | `1712177000` |

---

## Message Types

```c
MSG_TYPE_SENSOR_DATA = 1      /* Sensor sends data to server */
MSG_TYPE_COMMAND = 2           /* Client sends command to server */
MSG_TYPE_RESPONSE = 3          /* Server responds to sensor/client */
MSG_TYPE_ALERT = 4            /* Server sends alert to client */
MSG_TYPE_QUERY = 5            /* Client requests data from server */
```

---

## Sensor Types

```c
SENSOR_TYPE_TEMPERATURE = 1    /* Temperature sensor (°C) */
SENSOR_TYPE_ENERGY = 2         /* Energy consumption (kWh) */
SENSOR_TYPE_VIBRATION = 3      /* Vibration level (1-100) */
```

### Acceptable Value Ranges

| Sensor Type | Min | Max | Unit |
|------------|-----|-----|------|
| Temperature | -50 | 150 | °C |
| Energy | 0 | 10000 | kWh |
| Vibration | 0 | 100 | Level |

---

## Response Message Format

When the server receives a sensor data message, it responds with:

```
STATUS_CODE:SENSOR_ID:TIMESTAMP:MESSAGE
```

**Example:**
```
0:42:1712177000:OK
```

| Field | Type | Description | Example |
|-------|------|-------------|---------|
| STATUS_CODE | int | Response status (0=OK, 1=INVALID_FORMAT, etc.) | `0` |
| SENSOR_ID | int | ID of sensor being responded to | `42` |
| TIMESTAMP | long | Server timestamp | `1712177000` |
| MESSAGE | string | Status message description | `OK` |

---

## Status Codes

```c
STATUS_OK = 0                  /* Message processed successfully */
STATUS_INVALID_FORMAT = 1      /* Message format is invalid */
STATUS_INVALID_SENSOR = 2      /* Sensor ID not recognized */
STATUS_OUT_OF_RANGE = 3        /* Sensor value outside acceptable range */
STATUS_SERVER_ERROR = 4        /* Server encountered an error */
```

---

## Protocol Flow Examples

### Example 1: Valid Temperature Sensor Reading

**Sensor sends:**
```
1:5:1:22.3:1712177000
```

**Server responds:**
```
0:5:1712177000:OK
```

**Meaning:** Sensor ID 5 (Temperature) reports 22.3°C at timestamp 1712177000. Server acknowledges with OK.

---

### Example 2: Out of Range Temperature

**Sensor sends:**
```
1:5:1:200.0:1712177000
```

**Server responds:**
```
3:5:1712177000:Sensor value out of range
```

**Meaning:** Temperature of 200°C exceeds maximum (150°C), so server rejects with status 3.

---

### Example 3: Invalid Sensor Type

**Sensor sends:**
```
1:10:99:50.0:1712177000
```

**Server responds:**
```
1:10:1712177000:Invalid message format
```

**Meaning:** Sensor type 99 doesn't exist, so server returns invalid format error.

---

## Implementation Details

### Parser Function
```c
int protocol_parse_message(const char *buffer, int buffer_len, protocol_message_t *message);
```

Parses incoming raw message and populates the `protocol_message_t` structure. Returns 0 on success, -1 on error.

### Validation
- **Format validation:** Checks message has all 5 required fields
- **Range validation:** Ensures sensor value is within acceptable limits
- **Type validation:** Verifies sensor type is known (1-3)

### Builder Functions
```c
int protocol_build_sensor_message(const protocol_message_t *message, char *buffer, int buffer_len);
int protocol_build_response(const protocol_response_t *response, char *buffer, int buffer_len);
```

These functions convert message structures into string format for transmission.

---

## Usage Example in Code

### Receiving and Parsing

```c
char buffer[1024];
protocol_message_t msg;

/* Receive message from socket */
int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

/* Parse the message */
if (protocol_parse_message(buffer, bytes_received, &msg) == 0) {
    printf("Received from sensor %d: %.2f %s\n",
        msg.sensor_id,
        msg.value,
        protocol_get_sensor_type_name(msg.sensor_type));
}
```

### Building and Sending Response

```c
protocol_response_t response;
char response_buffer[256];

response.status = STATUS_OK;
response.sensor_id = msg.sensor_id;
response.timestamp = time(NULL);
strcpy(response.status_message, "OK");

int response_len = protocol_build_response(&response, response_buffer, sizeof(response_buffer));
send(client_fd, response_buffer, response_len, 0);
```

---

## Security Considerations

1. **Input Validation:** The parser validates all fields before accepting messages
2. **Buffer Overflow Prevention:** All string operations check buffer lengths
3. **Range Checking:** Sensor values must be within acceptable ranges
4. **Checksum (Optional):** Support for CRC-like checksums via `protocol_calculate_checksum()`

---

## Future Extensions

The protocol can be extended with:
- **Compression:** For large sensor networks
- **Encryption:** TLS/SSL wrapper for security
- **Batch Messages:** Multiple readings in one transmission
- **Acknowledgment IDs:** For reliable delivery tracking
- **Sensor Calibration:** Per-sensor calibration data
