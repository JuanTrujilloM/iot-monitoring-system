#include "server.h"
#include "sensor_manager.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    char *end = NULL;
    long port_long;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <logFile>\n", argv[0]);
        return EXIT_FAILURE;
    }

    port_long = strtol(argv[1], &end, 10);
    if (end == argv[1] || *end != '\0' || port_long < 1 || port_long > 65535 || port_long > INT_MAX) {
        fprintf(stderr, "Error: Invalid port (%s). Must be between 1 and 65535.\n", argv[1]);
        return EXIT_FAILURE;
    }

    /* The sensor manager is initialized before the server starts. */
    if (sensor_manager_init() != 0) {
        fprintf(stderr, "Error: Could not initialize sensor manager\n");
        return EXIT_FAILURE;
    }

    {
        int result = start_server((int)port_long, argv[2]);

        /* Cleaning up after termination (although the server usually doesn't terminate) */
        sensor_manager_cleanup();

        if (result != EXIT_SUCCESS) {
            fprintf(stderr, "Server finished with errors.\n");
        }

        return result;
    }
}