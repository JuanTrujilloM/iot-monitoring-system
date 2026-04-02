#include "server.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	char *fin = NULL;
	long puerto_largo;

	if (argc != 3) {
		fprintf(stderr, "Uso: %s <puerto> <archivoLogs>\n", argv[0]);
		return EXIT_FAILURE;
	}

	puerto_largo = strtol(argv[1], &fin, 10);
	if (fin == argv[1] || *fin != '\0' || puerto_largo < 1 || puerto_largo > 65535 || puerto_largo > INT_MAX) {
		fprintf(stderr, "Error: Puerto invalido (%s). Debe estar entre 1 y 65535.\n", argv[1]);
		return EXIT_FAILURE;
	}

	{
		int resultado = iniciar_servidor((int)puerto_largo, argv[2]);

		if (resultado != EXIT_SUCCESS) {
			fprintf(stderr, "El servidor finalizo con errores.\n");
		}

		return resultado;
	}
}
