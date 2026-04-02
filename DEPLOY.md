# Instrucciones de Despliegue

Ver guía detallada en [docs/aws-setup.md](docs/aws-setup.md).

## Despliegue rápido (local con Docker Compose)

```bash
docker-compose up --build
```

## Despliegue en AWS (resumen)

```bash
# 1. Construir imagen
docker build -t iot-server .

# 2. Ejecutar contenedor
docker run -d -p 8080:8080 -p 8081:8081 \
  -v $(pwd)/logs:/app/logs \
  iot-server 8080 logs/server.log

# 3. Ejecutar auth-service
cd auth-service && python3 auth_server.py

# 4. Ejecutar sensores (desde otra máquina o localmente)
cd sensors && python3 run_sensors.py --host iot-monitoring.example.com

# 5. Abrir cliente operador (Java)
cd operator-client && mvn package && java -jar target/operator-client-1.0-SNAPSHOT.jar
```

## DNS

Dominio configurado en Route 53: `TODO: completar con dominio real`
Apunta a IP pública EC2: `TODO: completar tras crear instancia`
