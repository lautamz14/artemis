<<<<<<< HEAD
#ifndef ARTEMIS_H
#define ARTEMIS_H

/* Datos de referencia del escenario 3 de la consigna. */
#define ARTEMIS_MASA_KG 26000.0
#define ARTEMIS_VELOCIDAD_TLI_MS 10900.0
#define ARTEMIS_ALTITUD_LEO_M 192000.0
#define ARTEMIS_DISTANCIA_TIERRA_LUNA_M 384000000.0

/* Constantes terrestres del escenario 2 de la consigna. */
#define ARTEMIS_G 6.674e-11
#define ARTEMIS_MASA_TIERRA_KG 5.972e24
#define ARTEMIS_RADIO_TIERRA_M 6371000.0

/* Dato adicional del modelo, NO proporcionado en el escenario 3. */
#define ARTEMIS_MASA_LUNA_KG 7.342e22

/* Cantidad maxima de eventos que puede almacenar el historial*/
#define ARTEMIS_MAX_EVENTOS 4

/* Aca preferi usar nombres para las etapas de simulacion para saber que representa
y como no se le asignan datos los asigna el programa con valores enteros comenzando desde el 0*/
typedef enum {
    ETAPA_PREPARACION,
    ETAPA_TLI,
    ETAPA_INERCIA,
    ETAPA_EVALUACION
} EtapaMision;

/* La estructura mision que es un registro basicamente para saber los estados o resultados de la mision y
justamente le puse el typedef para que le pueda llamar mision */
typedef struct {
    double masa_kg;
    double velocidad_base_ms;
    double ajuste_tli_ms;
    double velocidad_final_ms;
    double energia_base_j;
    double energia_final_j;
    double incremento_energia_j;
    double distancia_equilibrio_m;
    double cambio_potencial_j_kg;
    double trabajo_adicional_j_kg;
    double velocidad_minima_ms;
    double velocidad_equilibrio_ms;
    /* El de abajo esta representado por valores logicos, le asigne un entero para que en todo el programa tomen valores 
    de 0 para falso y 1 verdadero, asi no tenemos que agregar otra cabecera que es la de los booleanos*/
    int alcanza_equilibrio;
    int modo_experimental;
    EtapaMision etapa;
    int historial[ARTEMIS_MAX_EVENTOS];
    int cantidad_eventos;
} Mision;

/* Funciones publicas del modulo de simulacion. */
void artemis_inicializar(Mision *estado, double masa, double velocidad,
                        int modo_experimental);
void artemis_ejecutar_tli(Mision *estado, double ajuste_velocidad);
int artemis_avanzar_evento(Mision *estado);
void artemis_mostrar_estado(const Mision *estado);
void artemis_mostrar_reporte(const Mision *estado);
void artemis_mostrar_fundamentos(void);

#endif
=======
// Parametros fisicos

#ifndef ARTEMIS_H
#define ARTEMIS_H

#define G 6.674e-11  // m^3 kg^-1 s^-2
#define M_TIERRA 5.972e24  // kg
#define M_LUNA 7.348e22  // kg
#define RADIO_TIERRA 6.371e6  // m

#define V_TLI 10900.0  // m/s
#define MASA_ORION 26000.0  // kg
#define D_TIERRA 384000000.0 // m
#define H_LEO 192000.0 // m
#define RADIO_LEO (RADIO_TIERRA + H_LEO)

// Estado de la nave
typedef struct {
  double masa; 
  double posicion;
  double velocidad;
} Nave;

// Estado de la simulacion
typedef struct {
  int turno;
  double energia_cinetica;
  double energia_potencial;
  double energia_mecanica;
  double distancia_recorrida;
  int llegada_a_luna;
} Simulacion;

void calcular_energia_cinetica(const Nave *nave, double *resultado);

void calcular_energia_potencial(const Nave *nave, double *resultado);

double calcular_punto_equilibrio(void)

double trabajo_por_unidad_masa(double r_inicial, double r_final);

void actualizar_turno(Nave *nave, Simulacion *sim);

void generar_reporte_final(const Nave *nave, const Simulacion *sim);

#endif /* ARTEMIS_H */

>>>>>>> 573f134 (Subiendo parametros fisicos y prototipo de funciones)
