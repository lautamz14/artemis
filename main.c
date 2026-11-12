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

    /* Logo ARTEMIS en ASCII */
    puts("================================================================");
    puts("    _    ____  _____ _____ __  __ ___ ____  ");
    puts("   / \\  |  _ \\|_   _| ____|  \\/  |_ _/ ___| ");
    puts("  / _ \\ | |_) | | | |  _| | |\\/| || |\\___ \\ ");
    puts(" / ___ \\|  _ <  | | | |___| |  | || | ___) |");
    puts("/_/   \\_\\_| \\_\\ |_| |_____|_|  |_|___|____/ ");
    puts("================================================================");
    puts("              SIMULADOR DE MISIONES ARTEMIS");
    puts("      ESCENARIO 3: INYECCION TRANSLUNAR - TLI E INERCIA");
    puts("                   EN TRAYECTORIA LIBRE");
    puts("            (PRIMERA LEY DE NEWTON Y ENERGIA)");
    puts("================================================================");
    puts("\nBienvenido al sistema ARTEMIS.");
    puts("Analiza la maniobra TLI de Orion, el vuelo sin propulsion");
    puts("y la energia mecanica en un modelo idealizado de Fisica I.");

    /* Mantiene activo el menu principal hasta que el usuario decida salir, por eso no se pasan
    parametros, hasta que haya alguna accion especifica que lo interrumpa */
    for (;;) {
        puts("\n[ MENU PRINCIPAL ]");
        puts("  1. Iniciar nueva mision");
        puts("  2. Consultar fundamentos fisicos");
        puts("  3. Salir");
        
        /* Valida la opacion ingresada y termina si se cierra la entrada digamos */
        if (!leer_entero_rango("\nSelecciona una opcion: ", 1, 3, &opcion)) {
            puts("\nEntrada finalizada. Cerrando ARTEMIS.");
            return 0;
        }

        switch (opcion) {
            case 1:
                puts("\n============================================================");
                puts("                  CONFIGURACION DE MISION");
                puts("============================================================");
                puts("Elige los parametros para la simulacion idealizada.");
                puts("  1. Mision de referencia (datos de la consigna)");
                puts("  2. Simulacion experimental (masa y rapidez)");
                puts("  3. Volver al menu principal");
                if (!leer_entero_rango("\nSelecciona una opcion: ", 1, 3, &modo)) {
                    puts("\nEntrada finalizada. Cerrando ARTEMIS.");
                    return 0;
                }
                if (modo == 3) {
                    break;
                }
                
                /* 1 - Usa los parametros de referencia. 2 - solcita valores experimentales */
                if (modo == 1) {
                    masa = ARTEMIS_MASA_KG;
                    velocidad = ARTEMIS_VELOCIDAD_TLI_MS;
                    puts("\n[ DATOS DE LA MISION DE REFERENCIA ]");
                    printf("  Masa de Orion:                %.0f kg\n", masa);
                    printf("  Altitud inicial LEO:         %.0f km\n",
                           ARTEMIS_ALTITUD_LEO_M / 1000.0);
                    printf("  Distancia Tierra-Luna:       %.0f km\n",
                           ARTEMIS_DISTANCIA_TIERRA_LUNA_M / 1000.0);
                    printf("  Rapidez post-TLI base:       %.0f m/s\n", velocidad);
                } else {
                    puts("\n[ PARAMETROS EXPERIMENTALES ]");
                    puts("Utiliza un punto para los decimales.");
                    if (!leer_real_rango("Masa [kg] (1000 a 100000): ",
                                         1000.0, 100000.0, &masa) ||
                        !leer_real_rango("Rapidez post-TLI base [m/s] (1000 a 20000): ",
                                         1000.0, 20000.0, &velocidad)) {
                        puts("\nEntrada finalizada. Cerrando ARTEMIS.");
                        return 0;
                    }
                }
                
                puts("\n[ ALCANCE ]");
                puts("Se usa un modelo gravitatorio Tierra-Luna simplificado;");
                puts("no se calcula una trayectoria orbital real.");
                /* Muestra el estado de la nave con los parametros seleccionados */
                artemis_inicializar(&estadoNave, masa, velocidad, modo == 2);

                /* Ciclo de la mision, hace que puedas avanzar, consultar estados, o volver al menu principal*/
                for (;;) {
                    artemis_mostrar_estado(&estadoNave);
                    puts("\n[ MENU DE MISION ]");
                    if (estadoNave.etapa == ETAPA_PREPARACION) {
                        puts("  1. Ejecutar maniobra TLI");
                    } else if (estadoNave.etapa == ETAPA_EVALUACION) {
                        puts("  1. Consultar fundamentos fisicos");
                    } else {
                        puts("  1. Avanzar al siguiente evento");
                    }
                    puts(estadoNave.etapa == ETAPA_EVALUACION
                         ? "  2. Ver reporte final" : "  2. Ver reporte parcial");
                    puts("  3. Regresar al menu principal");
                    if (!leer_entero_rango("\nSelecciona una opcion: ",
                                           1, 3, &seleccion)) {
                        puts("\nEntrada finalizada. Cerrando ARTEMIS.");
                        return 0;
                    }

                    /* Ejecuta la TLI si se esta en preparacion y sino se analiza el evento */
                    if (seleccion == 1) {
                        if (estadoNave.etapa == ETAPA_PREPARACION) {
                            puts("\n[ DECISION DEL OPERADOR - MANIOBRA TLI ]");
                            puts("La ICPS proporciona la rapidez de inyeccion translunar.");
                            puts("Elige un ajuste ideal DURANTE la TLI, antes de apagar");
                            puts("los motores y desacoplar la etapa superior:");
                            puts("  1. Sin ajuste          (+0 m/s)");
                            puts("  2. Ajuste pequeno      (+10 m/s)");
                            puts("  3. Ajuste mayor        (+50 m/s)");
                            if (!leer_entero_rango("\nSelecciona una opcion: ",
                                                   1, 3, &ajuste_elegido)) {
                                puts("\nEntrada finalizada. Cerrando ARTEMIS.");
                                return 0;
                            }
                            /* Convierte la opcion elegida en un ajuste de velocidad, es como un condicional anidado, me 
                            parecio interesante para probarlo a ver q onda, se llama operacion ternaria algo asi */
                            ajuste = ajuste_elegido == 1 ? 0.0 :
                                     (ajuste_elegido == 2 ? 10.0 : 50.0);
                            artemis_ejecutar_tli(&estadoNave, ajuste);
                        } else if (estadoNave.etapa == ETAPA_EVALUACION) {
                            artemis_mostrar_fundamentos();
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
                puts("\nGracias por utilizar ARTEMIS. Fin del programa.");
                return 0;
        }
    }
}
