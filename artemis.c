#include "artemis.h"

#include <math.h>
#include <stdio.h>

/* Modelo didactico: Tierra y Luna fijas, representadas como masas puntuales.
   El eje Tierra-Luna es auxiliar para analizar gravedad y energia;
   NO constituye una trayectoria orbital de Orion. */
   static double potencial_especifico(double distancia_tierra_m)
{
    double distancia_luna_m = ARTEMIS_DISTANCIA_TIERRA_LUNA_M - distancia_tierra_m;

    return -ARTEMIS_G * ARTEMIS_MASA_TIERRA_KG / distancia_tierra_m
           - ARTEMIS_G * ARTEMIS_MASA_LUNA_KG / distancia_luna_m;
}

/* Calcula la distancia desde la Tierra hasta el punto
   donde se equilibran las fuerzas gravitatorias
   de la Tierra y la Luna en el modelo idealizado. */
   static double calcular_equilibrio(void)
{
    return ARTEMIS_DISTANCIA_TIERRA_LUNA_M /
           (1.0 + sqrt(ARTEMIS_MASA_LUNA_KG / ARTEMIS_MASA_TIERRA_KG));
}

static void registrar_evento(Mision *estado, int codigo)
{
    if (estado->cantidad_eventos < ARTEMIS_MAX_EVENTOS) {
        estado->historial[estado->cantidad_eventos] = codigo;
        estado->cantidad_eventos++;
    }
}

static const char *nombre_evento(int codigo)
{
    switch (codigo) {
        case 0: return "Preparacion de Orion";
        case 1: return "Maniobra TLI y desacople";
        case 2: return "Tramo sin propulsion principal";
        case 3: return "Evaluacion energetica";
        default: return "Evento desconocido";
    }
}

/* Encabezado de ancho fijo para los tres eventos de la mision. */
static void mostrar_encabezado_evento(const char *titulo)
{
    const int ancho = 60;
    int largo = (int)strlen(titulo);
    int izquierda = (ancho - largo) / 2;
    int derecha = ancho - largo - izquierda;

    puts("\n+------------------------------------------------------------+");
    printf("|%*s%s%*s|\n", izquierda, "", titulo, derecha, "");
    puts("+------------------------------------------------------------+");
}

void artemis_inicializar(Mision *estado, double masa, double velocidad,
                        int modo_experimental)
{
    int i;

    estado->masa_kg = masa;
    estado->velocidad_base_ms = velocidad;
    estado->ajuste_tli_ms = 0.0;
    estado->velocidad_final_ms = velocidad;
    estado->energia_base_j = 0.5 * masa * velocidad * velocidad;
    estado->energia_final_j = 0.0;
    estado->incremento_energia_j = 0.0;
    estado->distancia_equilibrio_m = calcular_equilibrio();
    estado->cambio_potencial_j_kg = 0.0;
    estado->deficit_energetico_j_kg = 0.0;
    estado->velocidad_minima_ms = 0.0;
    estado->supera_umbral_energetico = 0;
    estado->modo_experimental = modo_experimental;
    estado->etapa = ETAPA_PREPARACION;
    estado->cantidad_eventos = 0;
    for (i = 0; i < ARTEMIS_MAX_EVENTOS; i++) {
        estado->historial[i] = 0;
    }
    registrar_evento(estado, 0);
}

void artemis_ejecutar_tli(Mision *estado, double ajuste_velocidad)
{
    double radio_inicial;
    double energia_especifica;
    double energia_restante;

    if (estado->etapa != ETAPA_PREPARACION) {
        puts("La maniobra TLI ya fue realizada.");
        return;
    }

    /* El ajuste se decide ANTES del desacople: despues no hay empuje. */
    /* Se registra el ajuste de velocidad elegido para la maniobra TLI. */
    estado->ajuste_tli_ms = ajuste_velocidad;
    estado->velocidad_final_ms = estado->velocidad_base_ms + ajuste_velocidad;
    /* Se calcula la energia cinetica final luego del ajuste de velocidad TLI. */
    estado->energia_final_j = 0.5 * estado->masa_kg *
                              estado->velocidad_final_ms * estado->velocidad_final_ms;
    estado->incremento_energia_j = estado->energia_final_j - estado->energia_base_j;

    radio_inicial = ARTEMIS_RADIO_TIERRA_M + ARTEMIS_ALTITUD_LEO_M;
    estado->cambio_potencial_j_kg =
        potencial_especifico(estado->distancia_equilibrio_m) -
        potencial_especifico(radio_inicial);

    /* Balance energetico: se compara la energia cinetica
   especifica disponible con el cambio de potencial. */
    energia_especifica = 0.5 * estado->velocidad_final_ms * estado->velocidad_final_ms;
    energia_restante = energia_especifica - estado->cambio_potencial_j_kg;

    estado->velocidad_minima_ms = sqrt(2.0 * estado->cambio_potencial_j_kg);
     /* Interpretacion A: deficit especifico POST-TLI frente al nivel de
       potencial del equilibrio estatico. No representa combustible. */
    estado->deficit_energetico_j_kg = fmax(0.0, -energia_restante);
    estado->supera_umbral_energetico = energia_restante >= 0.0;
   
    mostrar_encabezado_evento("EVENTO 1/3 - MANIOBRA TLI");
    puts("\n[ SITUACION ]");
    puts("La etapa superior ICPS acaba de realizar la maniobra de");
    puts("inyeccion translunar de Orion desde la orbita baja terrestre.");
    puts("\n[ RESULTADOS DE LA MANIOBRA ]");
    printf("  Ajuste seleccionado durante la TLI:  +%.0f m/s\n",
           estado->ajuste_tli_ms);
    printf("  Rapidez obtenida por Orion:           %.2f m/s\n",
           estado->velocidad_final_ms);
    printf("  Energia cinetica post-TLI:            %.5e J\n",
           estado->energia_final_j);
    puts("\n[ INTERPRETACION FISICA ]");
    printf("La energia cinetica de Orion al finalizar la TLI es de\n"
           "%.5e joules. Representa la energia de movimiento inicial\n"
           "que consideramos en este analisis.\n", estado->energia_final_j);
    puts("En el modelo idealizado, al alejarse de la Tierra aumenta");
    puts("la energia potencial gravitatoria. Sin empuje posterior,");
    puts("parte de la energia cinetica puede transformarse en potencial.");
    puts("\n[ ESTADO ]");
    puts("  Maniobra TLI: COMPLETADA");
    puts("  Etapa ICPS:   DESACOPLADA");
    puts("  Motores:      APAGADOS");
    puts("--------------------------------------------------------------");

    estado->etapa = ETAPA_TLI;
    registrar_evento(estado, 1);
}

int artemis_avanzar_evento(Mision *estado)
{
    double radio_inicial;
    double distancia_luna;
    double aceleracion_neta;

    if (estado->etapa == ETAPA_TLI) {
        radio_inicial = ARTEMIS_RADIO_TIERRA_M + ARTEMIS_ALTITUD_LEO_M;
        distancia_luna = ARTEMIS_DISTANCIA_TIERRA_LUNA_M - radio_inicial;
        aceleracion_neta =
            -ARTEMIS_G * ARTEMIS_MASA_TIERRA_KG / (radio_inicial * radio_inicial)
            + ARTEMIS_G * ARTEMIS_MASA_LUNA_KG / (distancia_luna * distancia_luna);

        mostrar_encabezado_evento("EVENTO 2/3 - VUELO SIN PROPULSION");
        puts("\n[ SITUACION ]");
        puts("La TLI ha terminado. Orion continua su movimiento sin");
        puts("propulsion principal, bajo la atraccion gravitatoria.");
        puts("\n[ ANALISIS GRAVITATORIO ]");
        printf("  Aceleracion inicial sobre el eje Tierra-Luna:\n"
               "  %.3f m/s^2 (signo negativo: hacia la Tierra)\n",
               aceleracion_neta);
        puts("\n[ INTERPRETACION FISICA ]");
        puts("La Primera Ley de Newton explica que Orion no necesita");
        puts("motores encendidos para conservar su movimiento por inercia.");
        puts("Sin embargo, la gravedad sigue ejerciendo una fuerza.");
        puts("Por la Segunda Ley de Newton, esa fuerza puede modificar");
        puts("la velocidad de la nave.");
        puts("En nuestro modelo sobre el eje Tierra-Luna, la atraccion");
        puts("terrestre domina inicialmente y la aceleracion indicada");
        puts("se opone al sentido idealizado de avance hacia la Luna.");
        puts("\n[ ALCANCE ]");
        puts("Este valor pertenece al modelo 1D, no a una orbita real.");
        puts("--------------------------------------------------------------");

        estado->etapa = ETAPA_INERCIA;
        registrar_evento(estado, 2);
        return 1;
    }

    if (estado->etapa == ETAPA_INERCIA) {
        mostrar_encabezado_evento("EVENTO 3/3 - BALANCE ENERGETICO");
        puts("\n[ OBJETIVO ]");
        puts("Comparar la energia disponible tras la TLI con el aumento");
        puts("del potencial gravitatorio hasta el equilibrio idealizado.");
        puts("\n[ DATOS DEL ANALISIS ]");
        printf("  Rapidez post-TLI:                   %.2f m/s\n",
               estado->velocidad_final_ms);
        printf("  Rapidez umbral ideal:               %.2f m/s\n",
               estado->velocidad_minima_ms);
        printf("  Energia cinetica disponible:        %.2f J/kg\n",
               0.5 * estado->velocidad_final_ms * estado->velocidad_final_ms);
        printf("  Aumento de energia potencial:       %.2f J/kg\n",
               estado->cambio_potencial_j_kg);
        puts("\n[ RESULTADO DEL MODELO ]");
        if (estado->supera_umbral_energetico) {
            double margen = 0.5 * estado->velocidad_final_ms *
                            estado->velocidad_final_ms -
                            estado->cambio_potencial_j_kg;
            puts("  >>> ENERGIA SUFICIENTE <<<");
            printf("  Margen energetico:                  %.2f J/kg\n", margen);
            puts("\n[ INTERPRETACION FISICA ]");
            puts("La energia cinetica inicial alcanza el nivel de potencial");
            puts("del punto de equilibrio del modelo simplificado.");
            puts("No se requiere aporte energetico adicional para ese umbral.");
        } else {
            puts("  >>> ENERGIA INSUFICIENTE <<<");
            printf("  Deficit energetico:                 %.2f J/kg\n",
                   estado->deficit_energetico_j_kg);
            puts("\n[ INTERPRETACION FISICA ]");
            puts("La energia cinetica inicial es menor que el aumento de");
            puts("energia potencial necesario para alcanzar ese nivel.");
            printf("Faltarian %.2f joules por cada kilogramo de nave.\n",
                   estado->deficit_energetico_j_kg);
            puts("Esa energia adicional habria tenido que suministrarse");
            puts("durante la TLI, antes del apagado de los motores.");
        }
        puts("El resultado no predice la trayectoria real de Artemis.");
        puts("Los calculos detallados estan disponibles en el reporte.");
        puts("--------------------------------------------------------------");

        estado->etapa = ETAPA_EVALUACION;
        registrar_evento(estado, 3);
        return 1;
    }

    if (estado->etapa == ETAPA_PREPARACION) {
        puts("Primero hay que ejecutar la TLI.");
    } else {
        puts("La evaluacion ya termino. Consulta el reporte o inicia otra mision.");
    }
    return 0;
}

void artemis_mostrar_estado(const Mision *estado)
{
    const char *etapa;

    switch (estado->etapa) {
        case ETAPA_PREPARACION: etapa = "Preparacion"; break;
        case ETAPA_TLI: etapa = "TLI completada"; break;
        case ETAPA_INERCIA: etapa = "Vuelo sin propulsion"; break;
        case ETAPA_EVALUACION: etapa = "Mision evaluada"; break;
        default: etapa = "Desconocida"; break;
    }

    printf("\n[ ORION | %s | %s | eventos %d/3 ]\n",
           estado->modo_experimental ? "experimental" : "referencia",
           etapa, estado->cantidad_eventos - 1);
}

void artemis_mostrar_reporte(const Mision *estado)
{
    int i;

    puts("\n============================================================");
    puts("                  REPORTE DE MISION ARTEMIS");
    puts("============================================================");
    puts("\n[ 01 - DATOS DE LA MISION ]");
    puts("  Escenario:                 3 - Inyeccion Translunar");
    puts("  Nave:                      Orion");
    printf("  Modo:                      %s\n",
           estado->modo_experimental ? "Experimental" : "Referencia");
    printf("  Masa de Orion:             %.0f kg\n", estado->masa_kg);
    printf("  Rapidez post-TLI base:     %.2f m/s\n", estado->velocidad_base_ms);

    puts("\n------------------------------------------------------------");
    puts("[ 02 - MANIOBRA TLI ]");
    if (estado->etapa == ETAPA_PREPARACION) {
        puts("  La TLI todavia no fue ejecutada.");
        puts("  El resto de los resultados esta pendiente.");
    } else {
        printf("  Ajuste elegido:            +%.0f m/s\n", estado->ajuste_tli_ms);
        printf("  Rapidez al finalizar TLI:  %.2f m/s\n", estado->velocidad_final_ms);
        printf("  Energia cinetica de Orion: %.5e J\n", estado->energia_final_j);
        if (estado->ajuste_tli_ms != 0.0) {
            printf("  Aumento por el ajuste:     %.5e J\n",
                   estado->incremento_energia_j);
        }
        puts("  La energia cinetica corresponde al movimiento de Orion");
        puts("  inmediatamente despues de la maniobra TLI.");
    }

    puts("\n------------------------------------------------------------");
    puts("[ 03 - ANALISIS GRAVITATORIO Y ENERGETICO ]");
    if (estado->etapa != ETAPA_EVALUACION) {
        puts("  Pendiente: completar los eventos de la mision.");
    } else {
        printf("  Equilibrio desde Tierra:   %.2f km\n",
               estado->distancia_equilibrio_m / 1000.0);
        printf("  Aumento del potencial:     %.2f J/kg\n",
               estado->cambio_potencial_j_kg);
        printf("  Energia disponible:        %.2f J/kg\n",
               0.5 * estado->velocidad_final_ms * estado->velocidad_final_ms);
        printf("  Rapidez umbral ideal:      %.2f m/s\n",
               estado->velocidad_minima_ms);
        puts("  El equilibrio es el punto del eje idealizado donde");
        puts("  se igualan las fuerzas gravitatorias Tierra-Luna.");
    }

    puts("\n------------------------------------------------------------");
    puts("[ 04 - RESULTADO ENERGETICO ]");
    if (estado->etapa != ETAPA_EVALUACION) {
        puts("  Resultado pendiente de evaluacion.");
    } else if (estado->supera_umbral_energetico) {
        double margen = 0.5 * estado->velocidad_final_ms *
                        estado->velocidad_final_ms -
                        estado->cambio_potencial_j_kg;
        puts("  >>> ENERGIA SUFICIENTE <<<");
        printf("  Margen del modelo:         %.2f J/kg\n", margen);
        puts("  La energia disponible supera el nivel de potencial");
        puts("  correspondiente al equilibrio gravitatorio idealizado.");
    } else {
        puts("  >>> ENERGIA INSUFICIENTE <<<");
        printf("  Deficit especifico:        %.2f J/kg\n",
               estado->deficit_energetico_j_kg);
        puts("  Este deficit es un aporte energetico hipotetico que");
        puts("  habria sido necesario antes de apagar los motores.");
    }

    puts("\n------------------------------------------------------------");
    puts("[ 05 - IDA Y REGRESO ]");
    puts("  Alejarse de la Tierra requiere elevar la energia");
    puts("  potencial gravitatoria. Al regresar, la gravedad");
    puts("  puede transformarla nuevamente en energia cinetica.");
    puts("  Por eso la explicacion del combustible es cualitativa.");

    puts("\n------------------------------------------------------------");
    puts("[ 06 - HISTORIAL DE EVENTOS ]");
    for (i = 0; i < estado->cantidad_eventos; i++) {
        printf("  [OK] %s\n", nombre_evento(estado->historial[i]));
    }

    puts("\n------------------------------------------------------------");
    puts("[ 07 - LIMITACIONES DEL MODELO ]");
    puts("  Se consideran Tierra y Luna fijas y un eje auxiliar.");
    puts("  No se calcula una trayectoria orbital de retorno libre,");
    puts("  ni se estima el consumo real de combustible.");
    puts("============================================================");
    puts("                      FIN DEL REPORTE");
    puts("============================================================");
}

void artemis_mostrar_fundamentos(void)
{
    puts("\n============================================================");
    puts("                     FUNDAMENTOS FISICOS");
    puts("============================================================");
    puts("\n[ INERCIA - PRIMERA LEY DE NEWTON ]");
    puts("Si no actuara fuerza neta, Orion conservaria su velocidad.");
    puts("Apagar los motores no detiene la nave: continua por inercia.");
    puts("\n[ GRAVEDAD - SEGUNDA LEY DE NEWTON ]");
    puts("El empuje termina, pero las fuerzas gravitatorias siguen");
    puts("actuando. La fuerza neta produce aceleracion: F = m * a.");
    puts("Sobre el eje Tierra-Luna sumamos las fuerzas de ambos cuerpos.");
    puts("\n[ ENERGIA CINETICA Y POTENCIAL ]");
    puts("La energia de movimiento es K = m * v^2 / 2.");
    puts("La energia potencial gravitatoria cambia con la posicion.");
    puts("Sin trabajo externo, la energia mecanica total se conserva.");
    puts("\n[ EQUILIBRIO Y DEFICIT ENERGETICO ]");
    puts("El equilibrio del modelo ocurre donde se igualan las fuerzas");
    puts("de la Tierra y de la Luna sobre el eje que las une.");
    puts("Comparamos la energia post-TLI con el potencial en ese punto.");
    puts("Un deficit es energia que habria faltado durante la TLI,");
    puts("antes del apagado; no se encienden motores despues.");
    puts("\n[ IDA Y REGRESO ]");
    puts("Al alejarse de la Tierra, aumenta la energia potencial.");
    puts("Al volver, la gravedad favorece la conversion de energia");
    puts("potencial en cinetica. No se estima combustible en kilogramos.");
    puts("\n[ ALCANCE ]");
    puts("Modelo didactico de Fisica I, no simulacion orbital real.");
    puts("============================================================");
