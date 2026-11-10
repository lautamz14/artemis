#include "artemis.h"

#include <math.h>
#include <stdio.h>

/* Modelo educativo 1D: Tierra y Luna quietas y masas puntuales. */
static double potencial_especifico(double distancia_tierra_m)
{
    double distancia_luna_m = ARTEMIS_DISTANCIA_TIERRA_LUNA_M - distancia_tierra_m;

    return -ARTEMIS_G * ARTEMIS_MASA_TIERRA_KG / distancia_tierra_m
           - ARTEMIS_G * ARTEMIS_MASA_LUNA_KG / distancia_luna_m;
}

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
    estado->trabajo_adicional_j_kg = 0.0;
    estado->velocidad_minima_ms = 0.0;
    estado->velocidad_equilibrio_ms = 0.0;
    estado->alcanza_equilibrio = 0;
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
    estado->ajuste_tli_ms = ajuste_velocidad;
    estado->velocidad_final_ms = estado->velocidad_base_ms + ajuste_velocidad;
    estado->energia_final_j = 0.5 * estado->masa_kg *
                              estado->velocidad_final_ms * estado->velocidad_final_ms;
    estado->incremento_energia_j = estado->energia_final_j - estado->energia_base_j;

    radio_inicial = ARTEMIS_RADIO_TIERRA_M + ARTEMIS_ALTITUD_LEO_M;
    estado->cambio_potencial_j_kg =
        potencial_especifico(estado->distancia_equilibrio_m) -
        potencial_especifico(radio_inicial);

    energia_especifica = 0.5 * estado->velocidad_final_ms * estado->velocidad_final_ms;
    energia_restante = energia_especifica - estado->cambio_potencial_j_kg;

    estado->velocidad_minima_ms = sqrt(2.0 * estado->cambio_potencial_j_kg);
    estado->trabajo_adicional_j_kg = fmax(0.0, -energia_restante);
    estado->alcanza_equilibrio = energia_restante >= 0.0;
    estado->velocidad_equilibrio_ms = estado->alcanza_equilibrio
        ? sqrt(2.0 * energia_restante) : 0.0;

    puts("\n[EVENTO 1] ICPS ejecuta la maniobra TLI.");
    printf("Ajuste decidido: +%.0f m/s\n", estado->ajuste_tli_ms);
    printf("Velocidad tras la TLI: %.2f m/s\n", estado->velocidad_final_ms);
    printf("Energia cinetica tras TLI: %.3e J\n", estado->energia_final_j);
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

        puts("\n[EVENTO 2] Orion avanza sin propulsion principal.");
        puts("La primera ley de Newton no elimina la gravedad: su velocidad cambia.");
        printf("Aceleracion neta inicial sobre el eje idealizado: %.6f m/s^2\n",
               aceleracion_neta);
        estado->etapa = ETAPA_INERCIA;
        registrar_evento(estado, 2);
        return 1;
    }

    if (estado->etapa == ETAPA_INERCIA) {
        puts("\n[EVENTO 3] Evaluacion energetica.");
        printf("Equilibrio idealizado: %.2f km desde el centro terrestre\n",
               estado->distancia_equilibrio_m / 1000.0);
        printf("Cambio de potencial: %.2f J/kg\n", estado->cambio_potencial_j_kg);
        if (estado->alcanza_equilibrio) {
            puts("Resultado: energia suficiente en el modelo 1D para ese punto.");
            printf("Rapidez idealizada en ese punto: %.2f m/s\n",
                   estado->velocidad_equilibrio_ms);
        } else {
            puts("Resultado: energia insuficiente en el modelo 1D.");
            printf("Trabajo especifico adicional minimo: %.2f J/kg\n",
                   estado->trabajo_adicional_j_kg);
        }
        puts("Esto NO demuestra llegada a la Luna ni retorno libre real.");
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


