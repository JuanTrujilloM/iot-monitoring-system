# Arquitectura del Sistema

## Componentes

| Componente | Lenguaje | Protocolo | Puerto |
|------------|----------|-----------|--------|
| Servidor central | C | TCP (IMP/1.0) | 8080 |
| HTTP interface | C | HTTP/1.0 | 8081 |
| Auth service | Python | TCP interno | 9000 |
| Sensores simulados | Python | TCP (IMP/1.0) | — |
| Cliente operador | Java (Swing) | TCP (IMP/1.0) | — |

## Decisiones de diseño

### ¿Por qué TCP para sensores?
Las mediciones deben llegar garantizadas al servidor; una medición perdida puede
enmascarar una condición anómala. Se usa SOCK_STREAM.

### ¿Por qué el auth-service está separado?
El requerimiento exige que el servidor NO almacene usuarios localmente.
El auth-service es el "servicio externo de identidad".

### Resolución de nombres
Todos los componentes usan `getaddrinfo()` (C) o `socket.getaddrinfo()` (Python)
o `InetAddress.getByName()` (Java). Ninguna IP está hardcodeada.

## Diagrama de secuencia (simplificado)

```
Sensor          Servidor C        Auth Service     Operador Java
  |                 |                  |                 |
  |--REGISTER------>|                  |                 |
  |<------OK--------|                  |                 |
  |--DATA---------->|                  |                 |
  |                 |--AUTH_CHECK----->|                 |
  |                 |<-----AUTH_OK-----|                 |
  |                 |                  |  AUTH---------->|
  |                 |<-----------------+--AUTH-----------|
  |                 |--OK rol----------|---------------->|
  |--DATA---------->|                  |                 |
  |                 |==ALERT broadcast=================>|
```
