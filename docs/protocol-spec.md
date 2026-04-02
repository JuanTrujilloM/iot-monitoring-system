# Especificación del Protocolo de Aplicación — IoT Monitoring Protocol (IMP/1.0)

## Generalidades

- **Capa:** Aplicación
- **Transporte:** TCP (SOCK_STREAM) — garantiza entrega ordenada y fiable
- **Codificación:** texto plano UTF-8
- **Delimitador de mensaje:** `\n` (salto de línea)
- **Puerto por defecto:** 8080

## Formato general

```
COMANDO [ARG1] [ARG2] [...]\n
```

## Mensajes del cliente al servidor

| Comando | Argumentos | Descripción |
|---------|-----------|-------------|
| `REGISTER SENSOR` | `<id> <tipo>` | Registra un sensor. Debe ser el primer mensaje tras conectar |
| `DATA` | `<sensor_id> <valor> <unidad>` | Envía una medición |
| `AUTH` | `<usuario> <password>` | Autenticación de un operador |
| `LIST SENSORS` | — | Solicita lista de sensores activos |
| `LIST ALERTS` | — | Solicita historial de alertas recientes |
| `GET MEASUREMENTS` | `<sensor_id> [n]` | Últimas n mediciones de un sensor (default n=10) |
| `PING` | — | Keepalive |

## Respuestas del servidor al cliente

| Respuesta | Descripción |
|-----------|-------------|
| `OK` | Confirmación genérica |
| `OK <payload>` | Confirmación con datos |
| `ERROR <codigo> <descripcion>` | Error (ver tabla de códigos) |
| `ALERT <sensor_id> <tipo> <valor>` | Notificación push de evento anómalo |
| `PONG` | Respuesta a PING |

## Códigos de error

| Código | Descripción |
|--------|-------------|
| 400 | Mensaje mal formado |
| 401 | No autorizado |
| 404 | Sensor no encontrado |
| 409 | Sensor ya registrado |
| 500 | Error interno del servidor |

## Tipos de alerta

| Tipo | Condición |
|------|-----------|
| `HIGH_TEMP` | Temperatura > 80°C |
| `LOW_TEMP` | Temperatura < -10°C |
| `HIGH_VIB` | Vibración > 9.0 m/s² |
| `HIGH_POWER` | Consumo > 500W |
| `LOW_POWER` | Consumo < 5W |

## Flujo típico — Sensor

```
Sensor → Servidor : REGISTER SENSOR temp-01 temperature\n
Servidor → Sensor : OK\n
Sensor → Servidor : DATA temp-01 23.5 celsius\n
Servidor → Sensor : OK\n
```

## Flujo típico — Operador

```
Operador → Servidor : AUTH operator1 s3cr3t\n
Servidor → Operador : OK operator\n
Operador → Servidor : LIST SENSORS\n
Servidor → Operador : OK temp-01,temperature,23.5,celsius;vib-01,vibration,...\n
--- (en cualquier momento) ---
Servidor → Operador : ALERT temp-01 HIGH_TEMP 87.3\n
```

## Notas de implementación

- El servidor nunca cierra la conexión de un sensor activo.
- Si un operador se desconecta, el servidor deja de enviarle alertas.
- Todos los nombres de host se resuelven por DNS; ningún cliente usa IPs hardcodeadas.
