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

---

# Appendix: Public Demo (Render, free tier)

The distributed layout above needs a host that exposes arbitrary TCP ports, an
EC2 instance, and clients running on someone's laptop. That is the right shape
for the project but a poor shape for a demo link.

`Dockerfile.demo` packs all four components into a single image and publishes
only the dashboard. Everything else — sensors, C server, auth service — talks
over loopback inside the container, so no TCP ingress is required and the whole
thing fits in one free web service.

```
┌─ container ─────────────────────────────────────────┐
│  sensors (Python) ──TCP:8080──▶ central-server (C)   │
│                                       │              │
│                                  TCP:9000            │
│                                       ▼              │
│                                auth-service (Python) │
│                                       ▲              │
│  dashboard (Spring Boot) ──TCP:8080/9000             │
└──────────────────────│──────────────────────────────┘
                   HTTP $PORT  ◀── the only public port
```

## Deploying

1. Push to `main`.
2. Render → **New +** → **Blueprint** → pick this repository. `render.yaml`
   configures the service; no environment variables need to be entered.
3. First build takes ~10 minutes (it compiles the C server and runs a Maven
   build). Subsequent deploys reuse the layer cache.

Sign in to the dashboard with any account from `auth-service/init_db.py`, for
example `admin` / `Admin@2024!`.

## What the demo image changes

| Concern | Distributed setup | Demo image |
|---|---|---|
| Components | 4 containers / hosts | 1 container |
| Public ports | 8080, 9000, 8090 | dashboard only, on `$PORT` |
| Sensors | started by hand, locally | 2 started automatically at boot |
| Auth database | built by `Dockerfile.auth` | built at image build, same way |

Two sensors (temperature and vibration) are enough to produce live readings and
to trip both the warning and critical alert thresholds. The full fleet would add
memory pressure for no extra demonstration value: the JVM is capped at
`-Xmx200m` and the container settles around **235 MB of the 512 MB** the free
plan allows.

## Configuration

Hosts and ports are read from the environment, defaulting to loopback so the
image runs with no configuration:

| Variable | Default | Used by |
|---|---|---|
| `PORT` | `8090` | dashboard HTTP port |
| `IOT_SERVER_HOST` / `IOT_SERVER_PORT` | `127.0.0.1` / `8080` | dashboard, sensors |
| `IOT_AUTH_HOST` / `IOT_AUTH_PORT` | `127.0.0.1` / `9000` | dashboard |
| `IOT_SERVER_USERNAME` / `IOT_SERVER_PASSWORD` | `admin` / `Admin@2024!` | dashboard's session with the C server |

Point these at a remote stack to run the dashboard or the sensors against the
EC2 deployment instead.

## Fixes made along the way

- **`server/src/server.c:378`** carried a stray `a` after a statement, so the C
  server did not compile. `Dockerfile.server` and the Makefile build were both
  broken by it.
- **`auth-service/users.db` is committed and stale** — it holds `admin` / `123`,
  and `init_db.py` inserts with `OR IGNORE`, so the stale row survives. The demo
  image copies only the scripts and rebuilds the database, matching what
  `Dockerfile.auth` already did. Consider gitignoring the file.
- **`application.properties` and `sensors/config.py` hardcoded a dead EC2
  hostname**, which made every clone fail until edited by hand.

## Known issue

Sensors occasionally log `ERROR 400 Missing arguments` or a truncated response
such as `ERROR SUREMENT_RECEIVED`. The client assumes one `recv()` returns
exactly one protocol message, so replies that arrive coalesced in the same TCP
segment are misparsed. Measurements still reach the server and alerts still
fire; this predates the demo image and is a framing bug in `sensors/sensor_base.py`.
