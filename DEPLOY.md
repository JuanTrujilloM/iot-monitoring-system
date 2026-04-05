# Deployment Guide — IoT Monitoring System

## Architecture

```
[Local - Sensors (Python)]    -->  AWS EC2 (Docker)
[Local - Operator (Java)]     -->    central-server :8080
                                     auth-service   :9000
```

The central server and authentication service run on AWS inside Docker containers.
Clients (sensors and operator) run locally and connect to the server over the Internet.

---

## Prerequisites

- AWS account with access to EC2 and Route 53
- Docker and Docker Compose installed on the EC2 instance
- Python 3 installed locally (for sensors)
- Maven and Java installed locally (for the operator)

---

## 1. Create the EC2 Instance

1. AWS Console → EC2 → Launch Instance
2. Configuration:
   - **AMI:** Ubuntu Server 24.04 LTS
   - **Instance type:** t2.micro
   - **Key pair:** create or use an existing one (save the `.pem` file)
3. Security Group — add the following inbound rules:

| Type | Port | Source |
|---|---|---|
| SSH | 22 | 0.0.0.0/0 |
| Custom TCP | 8080 | 0.0.0.0/0 |
| Custom TCP | 9000 | 0.0.0.0/0 |

---

## 2. Connect to the Instance

From the browser: **EC2 → Instances → Connect → EC2 Instance Connect → Connect**

Or via SSH from your local terminal:
```bash
chmod 400 iot-system.pem
ssh -i iot-system.pem ubuntu@<EC2-PUBLIC-IP>
```

---

## 3. Install Docker on EC2

```bash
sudo apt-get update
sudo apt-get install -y docker.io docker-compose-v2
sudo systemctl enable docker
sudo systemctl start docker
sudo usermod -aG docker ubuntu
```

Log out and reconnect for the docker group to take effect.

---

## 4. Clone the Repository and Start the Containers

```bash
git clone https://github.com/JuanTrujilloM/iot-monitoring-system.git
cd iot-monitoring-system
docker compose up --build -d
```

Verify both containers are running:
```bash
docker ps
```

You should see `central-server` (port 8080) and `auth-service` (port 9000) with status `Up`.

---

## 5. DNS Configuration

The public DNS hostname automatically assigned by AWS to the EC2 instance is used:

```
ec2-3-80-62-98.compute-1.amazonaws.com
```

This name resolves to the public IP `3.80.62.98` without requiring an external domain.

An A record was also configured in **AWS Route 53**:
- **Hosted zone:** iot-monitoring.com
- **Record:** server → 3.80.62.98

---

## 6. Run the Sensors (Local)

```bash
cd sensors
python3 run_sensors.py --id sensor-temp   --type temperature --count 2 &
python3 run_sensors.py --id sensor-energy --type energy      --count 2 &
python3 run_sensors.py --id sensor-vib    --type vibration   --count 1 &
```

Launches 5 simultaneous sensors connected to the server on AWS.

To stop them:
```bash
pkill -f run_sensors.py
```

---

## 7. Run the Operator Web Client (Local)

```bash
cd operator-client
mvn spring-boot:run
```

Open the browser at `http://localhost:8090` and log in with the credentials configured in the auth-service.

---

## 8. Shut Down the Server

From the EC2 terminal:
```bash
cd iot-monitoring-system
docker compose down
```

---

## Verify Server Logs

```bash
docker logs central-server
```

Logs include: client IP, source port, received message, sent response, and generated alerts.
