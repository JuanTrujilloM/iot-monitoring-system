# Sistema Distribuido de Monitoreo de Sensores IoT

Proyecto Nro1 — Internet: Arquitectura y Protocolos

## Estructura

```
iot-monitoring-system/
├── server/            ← Servidor central (C + Berkeley Sockets)
├── sensors/           ← Sensores simulados (Python)
├── operator-client/   ← Cliente operador con GUI (Java/Swing)
├── auth-service/      ← Servicio de autenticación externo (Python)
├── docs/              ← Documentación técnica
├── Dockerfile         ← Imagen Docker del servidor C
├── docker-compose.yml ← Entorno de desarrollo local
├── PROTOCOL.md        ← Resumen del protocolo de aplicación
└── DEPLOY.md          ← Instrucciones de despliegue en AWS
```

## Componentes y responsables

| Componente | Lenguaje | Issues asignados |
|------------|----------|-----------------|
| Servidor C | C | HU-01, HU-02, HU-03, HU-04 (David, Mar) |
| Sensores Python | Python | HU-06, HU-07, HU-08, HU-09, HU-10, HU-11 (JuanTrujilloM) |
| Cliente Java | Java | HU-12, HU-13, HU-14, HU-15, HU-16 (AndresPerezQ) |
| Auth service | Python | HU-17, HU-18 (AndresPerezQ) |
| Docker + AWS + DNS | — | HU-20, HU-21, HU-22 |

## Inicio rápido (desarrollo local)

```bash
# Levantar servidor + auth-service
docker-compose up --build

# Lanzar sensores (en otra terminal)
cd sensors && python3 run_sensors.py

# Lanzar cliente operador
cd operator-client && mvn package && java -jar target/operator-client-1.0-SNAPSHOT.jar
```

## Protocolo

Ver [PROTOCOL.md](PROTOCOL.md) y [docs/protocol-spec.md](docs/protocol-spec.md).

## Despliegue en AWS

Ver [DEPLOY.md](DEPLOY.md) y [docs/aws-setup.md](docs/aws-setup.md).
