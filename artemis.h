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

