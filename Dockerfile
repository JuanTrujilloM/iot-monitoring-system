# Dockerfile - Servidor central de monitoreo IoT (C)
#
# Etapa de compilación
FROM gcc:12 AS builder

WORKDIR /app
COPY server/ ./server/

RUN make -C server/

# Etapa de ejecución (imagen más liviana)
FROM debian:bookworm-slim

WORKDIR /app

COPY --from=builder /app/server/server ./server
RUN mkdir -p logs

# Puerto del protocolo principal y puerto HTTP
EXPOSE 8080 8081

# Uso: docker run ... <puerto> <archivoDeLogs>
# El ENTRYPOINT acepta argumentos externos para cumplir: ./server puerto archivoDeLogs
ENTRYPOINT ["./server"]
CMD ["8080", "logs/server.log"]
