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
    estado->alcanza_equilibrio = energia_restante >= 0.0;
   
     puts("\n[EVENTO 1] ICPS ejecuta la maniobra TLI.");
    printf("Ajuste ideal previo al apagado: +%.0f m/s\n", estado->ajuste_tli_ms);
    printf("Rapidez post-TLI: %.2f m/s\n", estado->velocidad_final_ms);
    printf("Energia cinetica post-TLI: %.3e J\n", estado->energia_final_j);
    puts("La etapa ICPS se desacopla y se apagan los motores principales.");

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

        puts("\n[EVENTO 2] Movimiento sin propulsion principal.");
        puts("La inercia permite continuar el movimiento sin empuje.");
        puts("La gravedad sigue actuando y modifica el vector velocidad.");
        printf("Campo neto inicial en el eje auxiliar Tierra-Luna: %.6f m/s^2\n",
               aceleracion_neta);
        puts("El signo negativo indica predominio de la atraccion terrestre.");
        puts("En una salida radial ideal, su gravedad se opone al avance y");
        puts("pierde intensidad a medida que aumenta la distancia terrestre.");
        puts("No es una desaceleracion orbital real calculada para Orion.");
        estado->etapa = ETAPA_INERCIA;
        registrar_evento(estado, 2);
        return 1;
    }

    if (estado->etapa == ETAPA_INERCIA) {
        puts("\n[EVENTO 3] Evaluacion energetica.");
        printf("Equilibrio de atracciones en el modelo estatico: %.2f km desde la Tierra\n",
               estado->distancia_equilibrio_m / 1000.0);
        printf("Aumento de potencial especifico: %.2f J/kg\n",
               estado->cambio_potencial_j_kg);
        printf("Energia cinetica especifica post-TLI: %.2f J/kg\n",
               0.5 * estado->velocidad_final_ms * estado->velocidad_final_ms);
        if (estado->supera_umbral_energetico) {
            puts("Resultado: energia suficiente para el nivel de potencial objetivo.");
        } else {
            puts("Resultado: energia insuficiente para el nivel de potencial objetivo.");
            printf("Deficit energetico especifico: %.2f J/kg\n",
                   estado->deficit_energetico_j_kg);
            puts("Ese aporte habria sido necesario antes del apagado.");
        }
        puts("El balance NO demuestra llegada a la Luna ni retorno libre real.");
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
    printf("\nOrion | %s | Masa: %.0f kg\n",
           estado->modo_experimental ? "modo experimental" : "modo referencia",
           estado->masa_kg);
    if (estado->etapa == ETAPA_PREPARACION) {
        printf("Rapidez post-TLI de referencia: %.2f m/s\n", estado->velocidad_base_ms);
        puts("Estado: esperando la maniobra TLI.");
    } else {
        printf("Rapidez post-TLI: %.2f m/s | Ajuste ideal: +%.0f m/s\n",
               estado->velocidad_final_ms, estado->ajuste_tli_ms);
        printf("Eventos completados: %d de 3\n", estado->cantidad_eventos - 1);
    }
}

void artemis_mostrar_reporte(const Mision *estado)
{
    int i;

    puts("\n============= REPORTE ARTEMIS =============");
    printf("Modo: %s\n", estado->modo_experimental ? "experimental" : "referencia");
    printf("Masa: %.2f kg\n", estado->masa_kg);
    printf("Rapidez post-TLI de referencia: %.2f m/s\n", estado->velocidad_base_ms);
    printf("Energia cinetica de referencia: %.6e J\n", estado->energia_base_j);

    if (estado->etapa == ETAPA_PREPARACION) {
        puts("Aun no se ejecuto la TLI. No hay evaluacion energetica.");
    } else {
        printf("Ajuste ideal previo al desacople: +%.0f m/s\n", estado->ajuste_tli_ms);
        printf("Rapidez post-TLI elegida: %.2f m/s\n", estado->velocidad_final_ms);
        printf("Energia cinetica post-TLI: %.6e J\n", estado->energia_final_j);
        printf("Incremento de energia cinetica por ajuste: %.6e J\n",
               estado->incremento_energia_j);
        printf("Equilibrio de atracciones (modelo estatico): %.2f km desde la Tierra\n",
               estado->distancia_equilibrio_m / 1000.0);
        printf("Aumento de potencial especifico: %.2f J/kg\n",
               estado->cambio_potencial_j_kg);
        printf("Rapidez umbral del modelo: %.2f m/s\n", estado->velocidad_minima_ms);
        printf("Energia cinetica especifica post-TLI: %.2f J/kg\n",
               0.5 * estado->velocidad_final_ms * estado->velocidad_final_ms);
        printf("Deficit energetico especifico (interpretacion A): %.2f J/kg\n",
               estado->deficit_energetico_j_kg);
        if (estado->etapa == ETAPA_EVALUACION) {
            printf("Balance energetico: %s\n", estado->supera_umbral_energetico
                   ? "energia suficiente para el nivel de potencial objetivo"
                   : "energia insuficiente para el nivel de potencial objetivo");
        } else {
            puts("Evaluacion final: pendiente de completar los eventos.");
        }
    }

    puts("Historial de eventos:");
    for (i = 0; i < estado->cantidad_eventos; i++) {
        printf("  %d. %s\n", i + 1, nombre_evento(estado->historial[i]));
    }
    puts("Interpretacion A: deficit post-TLI, aporte hipotetico previo al apagado.");
    puts("Limites: eje auxiliar Tierra-Luna y cuerpos fijos; sin trayectoria orbital");
    puts("real, retorno libre calculado ni consumo de combustible.");
    puts("===========================================");
}

void artemis_mostrar_fundamentos(void)
{
    puts("\n============= FUNDAMENTOS =============");
    puts("a) Inercia: sin empuje la nave sigue moviendose, pero la gravedad");
    puts("   ejerce una fuerza y cambia su vector velocidad (leyes de Newton).");
    puts("   En la salida radial ideal, la gravedad terrestre se opone al");
    puts("   alejamiento y su intensidad disminuye con la distancia.");
    puts("b) Energia cinetica post-TLI: K = m * v^2 / 2.");
    puts("c) Modelo: Tierra y Luna fijas. Calculamos donde sus atracciones");
    puts("   se igualan, usando el eje Tierra-Luna solo como referencia.");
    puts("   El cambio de potencial especifico es delta_phi = phi_final - phi_inicial.");
    puts("   Deficit = max(0, delta_phi - v_post_TLI^2 / 2), en J/kg.");
    puts("   Es energia que habria faltado aportar ANTES del apagado, no");
    puts("   una maniobra que el programa realiza despues de la TLI.");
    puts("d) En la ida se necesita energia para alejarse de la Tierra;");
    puts("   en el regreso la gravedad puede transformar potencial en cinetica.");
    puts("   No es posible deducir la masa de combustible con estos datos.");
    puts("Los ajustes son ideales y no representan consumo real de combustible.");
    puts("El programa NO calcula una orbita ni una trayectoria de retorno libre.");
    puts("=========================================");
}
