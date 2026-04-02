# Protocolo de Aplicación — Resumen ejecutivo

Ver especificación completa en [docs/protocol-spec.md](docs/protocol-spec.md).

## Protocolo: IMP/1.0 (IoT Monitoring Protocol)

- Basado en texto, delimitado por `\n`
- Transporte: TCP (SOCK_STREAM)
- Puerto principal: **8080**

## Comandos principales

```
REGISTER SENSOR <id> <tipo>
DATA <sensor_id> <valor> <unidad>
AUTH <usuario> <password>
LIST SENSORS
LIST ALERTS
GET MEASUREMENTS <sensor_id> [n]
PING
```

## Respuestas

```
OK [payload]
ERROR <codigo> <descripcion>
ALERT <sensor_id> <tipo> <valor>
PONG
```
