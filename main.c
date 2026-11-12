#include "artemis.h"
#include "validaciones.h"

#include <stdio.h>

/* main solo coordina los menus, artemis.c realiza la simulacion. */
int main(void)
{
    Mision estadoNave;
    double masa;
    double velocidad;
    double ajuste;
    int opcion;
    int modo;
    int seleccion;
    int ajuste_elegido;

    puts("====================================");
    puts("     ARTEMIS | ESCENARIO 3 - TLI");
    puts("====================================");

    /* Mantiene activo el menu principal hasta que el usuario decida salir, por eso no se pasan
    parametros, hasta que haya alguna accion especifica que lo interrumpa */
    for (;;) {
        puts("\nMENU PRINCIPAL");
        puts("1. Iniciar nueva mision");
        puts("2. Fundamentos fisicos");
        puts("3. Salir");
        
        /* Valida la opacion ingresada y termina si se cierra la entrada digamos */
        if (!leer_entero_rango("Opcion: ", 1, 3, &opcion)) {
            puts("\nEntrada finalizada. Cerrando Artemis.");
            return 0;
        }

        switch (opcion) {
            case 1:
                puts("\nCONFIGURACION DE MISION");
                puts("Balance energetico idealizado: no se calcula una orbita real.");
                puts("1. Mision de referencia (datos de la consigna)");
                puts("2. Simulacion experimental (elegir masa y velocidad)");
                puts("3. Volver al menu principal");
                if (!leer_entero_rango("Opcion: ", 1, 3, &modo)) {
                    puts("\nEntrada finalizada. Cerrando Artemis.");
                    return 0;
                }
                if (modo == 3) {
                    break;
                }
                
                /* 1 - Usa los parametros de referencia. 2 - solcita valores experimentales */
                if (modo == 1) {
                    masa = ARTEMIS_MASA_KG;
                    velocidad = ARTEMIS_VELOCIDAD_TLI_MS;
                    printf("\nReferencia: %.0f kg y rapidez post-TLI de %.0f m/s.\n", masa, velocidad);
                } else {
                    puts("\nUtiliza un punto para los decimales.");
                    if (!leer_real_rango("Masa [kg] (1000 a 100000): ",
                                         1000.0, 100000.0, &masa) ||
                        !leer_real_rango("Rapidez post-TLI base [m/s] (1000 a 20000): ",
                                         1000.0, 20000.0, &velocidad)) {
                        puts("\nEntrada finalizada. Cerrando Artemis.");
                        return 0;
                    }
                }
                
                /* Muestra el estado de la nave con los parametros seleccionados */
                artemis_inicializar(&estadoNave, masa, velocidad, modo == 2);

                /* Ciclo de la mision, hace que puedas avanzar, consultar estados, o volver al menu principal*/
                for (;;) {
                    artemis_mostrar_estado(&estadoNave);
                    puts("\nMENU DE MISION");
                    if (estadoNave.etapa == ETAPA_PREPARACION) {
                        puts("1. Ejecutar maniobra TLI");
                    } else {
                        puts("1. Avanzar al siguiente evento");
                    }
                    puts("2. Ver reporte parcial o final");
                    puts("3. Regresar al menu principal");
                    if (!leer_entero_rango("Opcion: ", 1, 3, &seleccion)) {
                        puts("\nEntrada finalizada. Cerrando Artemis.");
                        return 0;
                    }

                    /* Ejecuta la TLI si se esta en preparacion y sino se analiza el evento */
                    if (seleccion == 1) {
                        if (estadoNave.etapa == ETAPA_PREPARACION) {
                            puts("\nDECISION PREVIA AL DESACOPLE");
                            puts("Elegir ajuste ideal de rapidez antes del apagado:");
                            puts("1. Sin ajuste (+0 m/s)");
                            puts("2. Ajuste pequeno (+10 m/s)");
                            puts("3. Ajuste mayor (+50 m/s)");
                            if (!leer_entero_rango("Opcion: ", 1, 3,
                                                   &ajuste_elegido)) {
                                puts("\nEntrada finalizada. Cerrando Artemis.");
                                return 0;
                            }
                            /* Convierte la opcion elegida en un ajuste de velocidad, es como un condicional anidado, me 
                            parecio interesante para probarlo a ver q onda, se llama operacion ternaria algo asi */
                            ajuste = ajuste_elegido == 1 ? 0.0 :
                                     (ajuste_elegido == 2 ? 10.0 : 50.0);
                            artemis_ejecutar_tli(&estadoNave, ajuste);
                        } else {
                            artemis_avanzar_evento(&estadoNave);
                        }
                    } else if (seleccion == 2) {
                        artemis_mostrar_reporte(&estadoNave);
                    } else {
                        break;
                    }
                }
                break;
                
                /* Casi todos los breaks hacen que se vuelva al menu principal o a de la seccion donde estan */
            case 2:
                artemis_mostrar_fundamentos();
                break;

            case 3:
                puts("Gracias por utilizar Artemis. Fin del programa.");
                return 0;
        }
    }
}
