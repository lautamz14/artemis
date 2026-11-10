#include "validaciones.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAM_LINEA 128

void mostrar_advertencia(const char *mensaje)
{
    printf("[AVISO] %s\n", mensaje);
}

/* 1: linea completa; 0: EOF; -1: entrada demasiado larga. */
static int leer_linea(const char *mensaje, char linea[], size_t capacidad)
{
    size_t longitud;
    int caracter;

    printf("%s", mensaje);
    fflush(stdout);

    if (fgets(linea, (int)capacidad, stdin) == NULL) {
        return 0;
    }

    longitud = strlen(linea);
    if (longitud > 0 && linea[longitud - 1] == '\n') {
        linea[longitud - 1] = '\0';
    } else if (!feof(stdin)) {
        /* Una linea sin salto puede estar incompleta: vaciarla. */
        do {
            caracter = getchar();
        } while (caracter != '\n' && caracter != EOF);
        return -1;
    }

    return 1;
}

static int solo_espacios(const char *texto)
{
    while (*texto != '\0') {
        if (!isspace((unsigned char)*texto)) {
            return 0;
        }
        texto++;
    }
    return 1;
}
