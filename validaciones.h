#ifndef VALIDACIONES_H
#define VALIDACIONES_H

/* Devuelven 1 si se leyo un valor valido y 0 si se cerro la entrada. */
int leer_entero_rango(const char *mensaje, int minimo, int maximo, int *valor);
int leer_real_rango(const char *mensaje, double minimo, double maximo, double *valor);
void mostrar_advertencia(const char *mensaje);

#endif
