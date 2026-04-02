/*
 * main.c - Entry point del servidor central de monitoreo IoT
 *
 * Uso: ./server <puerto> <archivoDeLogs>
 */

#include <stdio.h>
#include <stdlib.h>
#include "../include/server.h"
#include "../include/logger.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <puerto> <archivoDeLogs>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int port = atoi(argv[1]);
    const char *logfile = argv[2];

    // TODO: inicializar logger
    // TODO: inicializar servidor (sockets, threads)
    // TODO: iniciar HTTP server en puerto secundario
    // TODO: bucle principal de aceptación de conexiones

    return EXIT_SUCCESS;
}
