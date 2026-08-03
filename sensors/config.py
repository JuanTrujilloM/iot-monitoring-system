import os

DEFAULT_HOST = os.getenv("IOT_SERVER_HOST", "127.0.0.1")
DEFAULT_PORT = int(os.getenv("IOT_SERVER_PORT", "8080"))
DEFAULT_INTERVAL = 5.0 # seconds between measurements
DEFAULT_RETRY_INTERVAL = 10.0 # seconds before reconnection attempt
