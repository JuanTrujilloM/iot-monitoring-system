FROM gcc:latest

WORKDIR /app

# Instalamos dependencias necesarias (si las hubiera)
RUN apt-get update && apt-get install -y build-essential

# Copiamos las carpetas de código
COPY src/ ./src/
COPY include/ ./include/

# Compilamos el servidor
RUN gcc src/*.c -I include -o server -lpthread

# Creamos carpeta para los logs
RUN mkdir -p logs

# Exponemos el puerto 8080
EXPOSE 8080

# Comando para ejecutar el servidor
CMD ["./server", "8080", "logs/server.log"]