# Proyecto Artemis — Escenario 3: Inyección Translunar (TLI) e Inercia

Simulador interactivo en lenguaje C (estándar **C11**) que modela física y energéticamente la maniobra de **Inyección Translunar (TLI)** de la nave espacial **Orion** impulsada por la etapa superior **ICPS**, su posterior tramo en vuelo inercial y la evaluación de alcance del punto de equilibrio gravitatorio Tierra-Luna mediante el principio de conservación de la energía mecánica.

---

## Tabla de Contenidos

1. [Descripción del Escenario Físico](#-descripción-del-escenario-físico)
2. [Arquitectura del Proyecto](#-arquitectura-del-proyecto)
3. [Análisis Técnico del Uso de Punteros](#-análisis-técnico-del-uso-de-punteros)
4. [Depuración de Errores y Validación Robusta de Entradas](#-depuración-de-errores-y-validación-robusta-de-entradas)
5. [Declaración de Uso de Inteligencia Artificial (IA)](#-declaración-de-uso-de-inteligencia-artificial-ia)
6. [Respuestas Analíticas a la Consigna](#-respuestas-analíticas-a-la-consigna)
7. [Compilación y Ejecución](#-compilación-y-ejecución)
8. [Límites del Modelo](#-límites-del-modelo)

---

## Descripción del Escenario Físico

Partiendo de una órbita terrestre baja (**LEO**, $h = 192\,000\text{ m}$), la etapa ICPS enciende su motor principal para llevar a la nave Orion ($m = 26\,000\text{ kg}$) hasta una velocidad de inyección translunar de referencia $v_{\text{TLI}} = 10\,900\text{ m/s}$ ($10.9\text{ km/s}$). Inmediatamente después, la etapa ICPS se desacopla y los motores se apagan por completo, iniciando el viaje hacia la Luna ubicada a una distancia media $D = 384\,000\,000\text{ m}$.

### Parámetros y Constantes del Sistema (`artemis.h`)

| Magnitud                           |     Símbolo      |   Valor en el Modelo    |                Unidad                 |
| :--------------------------------- | :--------------: | :---------------------: | :-----------------------------------: |
| Masa de referencia de Orion        |       $m$        |       $26\,000.0$       |              $\text{kg}$              |
| Velocidad base de inyección (TLI)  | $v_{\text{TLI}}$ |       $10\,900.0$       |             $\text{m/s}$              |
| Altitud de órbita baja (LEO)       | $h_{\text{LEO}}$ |      $192\,000.0$       |              $\text{m}$               |
| Distancia media Tierra-Luna        |       $D$        |   $3.84 \times 10^8$    |              $\text{m}$               |
| Constante de gravitación universal |       $G$        | $6.674 \times 10^{-11}$ | $\text{N}\cdot\text{m}^2/\text{kg}^2$ |
| Masa de la Tierra                  |      $M_T$       | $5.972 \times 10^{24}$  |              $\text{kg}$              |
| Radio medio terrestre              |      $R_T$       |     $6\,371\,000.0$     |              $\text{m}$               |
| Masa de la Luna                    |      $M_L$       | $7.342 \times 10^{22}$  |              $\text{kg}$              |

---

## Arquitectura del Proyecto

El programa está modularizado bajo el principio de separación de responsabilidades:

* **`main.c`**: Punto de entrada y coordinador de la interfaz de usuario por consola. Administra los menús mediante bucles controlados (`for (;;)`) y aloja en su pila (*stack*) la estructura principal `Mision estadoNave`.
* **`artemis.h` / `artemis.c`**: Motor físico y lógico de la simulación. Define el registro `Mision`, la enumeración `EtapaMision` y las funciones que calculan el potencial gravitatorio específico, el punto de equilibrio, el déficit energético (`deficit_energetico_j_kg`), la evaluación del umbral (`supera_umbral_energetico`) y la evolución por eventos.
* **`validaciones.h` / `validaciones.c`**: Módulo defensivo de lectura desde la entrada estándar (`stdin`). Previene bucles infinitos y desbordamientos de búfer (*buffer overflow*), limpiando entradas residuales y validando rangos numéricos mediante `strtol` y `strtod`.
* **`Makefile`**: Script de construcción automatizada multiplataforma (Linux/macOS/Windows) con banderas estrictas de compilación (`-std=c11 -Wall -Wextra -Wpedantic -O2`).

---

## Análisis Técnico del Uso de Punteros

El proyecto evita por completo el uso de variables globales y emplea punteros en C de forma estructurada en **cuatro escenarios clave**:

### 1. Paso por Referencia de la Estructura Principal (`Mision *estado`)

La estructura `Mision` concentra todos los parámetros y resultados de la simulación. En lugar de copiar toda la estructura por valor en cada llamada, `estadoNave` se instancia una única vez en `main()` y se pasa su dirección de memoria (`&estadoNave`):

```text
[ Memoria Stack de main() ]               [ Funciones en artemis.c ]
+-----------------------------------+     +--------------------------+
| Mision estadoNave                 | <-- | Mision *estado           |
|  ├─ masa_kg: 26000.0              |     | (Recibe &estadoNave y    |
|  ├─ velocidad_base_ms: 10900.0    |     |  modifica los campos     |
|  ├─ deficit_energetico_j_kg       |     |  mediante el operador    |
|  ├─ supera_umbral_energetico      |     |  flecha: estado->campo)  |
|  ├─ etapa: ETAPA_PREPARACION      |     +--------------------------+
|  └─ historial[4]                  |
+-----------------------------------+
```

* **Funciones modificadoras**: `artemis_inicializar()`, `artemis_ejecutar_tli()`, `artemis_avanzar_evento()` y `registrar_evento()` reciben `Mision *estado` y actualizan directamente la estructura original en `main`.

### 2. Punteros Constantes para Solo Lectura (`const Mision *estado`)

En `artemis_mostrar_estado(const Mision *estado)` y `artemis_mostrar_reporte(const Mision *estado)`, el calificador `const` garantiza que las funciones de visualización accedan eficientemente a los datos por dirección sin permitir que el estado físico de la misión sea alterado por error dentro de los reportes.

### 3. Parámetros de Salida por Puntero (`int *valor`, `double *valor`)

Las funciones `leer_entero_rango()` y `leer_real_rango()` utilizan su valor de retorno (`int`) para informar si la lectura fue exitosa (`1`) o si se cerró el flujo de entrada (`0`), mientras que depositan el número validado directamente en las variables locales de `main` (`&opcion`, `&modo`, `&masa`, `&velocidad`) mediante desreferenciación (`*valor = resultado;`).

### 4. Puntero Doble (`char **`) y Aritmética de Punteros en Cadenas (`validaciones.c`)

En la validación de texto ingresado por consola se combinan dos niveles de manejo de punteros:
* **Puntero a puntero (`&fin`)**: Se pasa la dirección del puntero `char *fin` a `strtol(linea, &fin, 10)` y `strtod(linea, &fin)`. Estas funciones actualizan `fin` para que apunte al primer carácter de `linea[]` donde terminó la conversión numérica.
* **Aritmética de punteros (`texto++`)**: La función `solo_espacios(const char *texto)` recibe el puntero `fin` y recorre el resto de la cadena incrementando la dirección de memoria (`texto++`) y evaluando cada carácter con `*texto` hasta encontrar el terminador nulo `'\0'`.

```text
Arreglo linea[]:  [ '1' ][ '0' ][ '9' ][ '0' ][ '0' ][ ' ' ][ '\0' ]
                     ^                                  ^
                     |                                  |
              linea (inicio)                     fin (apunta aquí tras strtod)
                                                  └──> solo_espacios() avanza con texto++
```

---

## Depuración de Errores y Validación Robusta de Entradas

Durante el desarrollo del proyecto se atravesaron dos grandes etapas de depuración (*debugging*): la corrección del flujo lógico-matemático del simulador y el blindaje de la lectura de datos por consola.

### 1. Depuración Sintáctica y Lógica en `artemis.c`

En las primeras versiones del módulo físico se detectaron y corrigieron mediante compilación estricta (`-Wall -Wextra -Wpedantic`) y pruebas de escritorio los siguientes errores:
* **Omisión del cálculo de potencial gravitatorio**: Inicialmente, la función `potencial_especifico()` estaba definida pero no era invocada dentro de `artemis_ejecutar_tli()`, provocando que `cambio_potencial_j_kg` permaneciera en `0.0` y que el déficit energético diera resultados nulos. Se corrigió calculando la diferencia de potencial entre la posición orbital inicial (`radio_inicial`) y el punto de equilibrio (`distancia_equilibrio_m`).
* **Consistencia de variables en el TAD `Mision`**: Se unificaron los nombres de los miembros de la estructura (`deficit_energetico_j_kg` y `supera_umbral_energetico`) entre la cabecera `artemis.h` y su implementación en `artemis.c`, utilizando banderas enteras (`0` y `1`) para representar estados lógicos sin depender de librerías adicionales.
* **Declaración de variables locales y tipos**: Se corrigieron variables locales no declaradas en `artemis_avanzar_evento()` (`radio_inicial`, `distancia_luna`, `aceleracion_neta`) y errores tipográficos en identificadores de macros y formatos de `printf`.

### 2. Blindaje de Entrada Estándar (`validaciones.c`) frente a `scanf`

En C a nivel introductorio suele utilizarse `scanf("%d", &opcion)`, pero esto genera fallos críticos (bucles infinitos) cuando el usuario ingresa letras, símbolos, comas en vez de puntos decimales o líneas de más de 128 caracteres. Para solucionarlo se implementó un sistema de depuración de entrada en tres capas:

1. **Lectura acotada con `fgets` y limpieza de búfer (`leer_linea`)**:
   Se lee un máximo de `TAM_LINEA` ($128$) caracteres. Si el usuario escribe una línea más larga que el búfer (no se encuentra el salto de línea `'\n'`), un bucle `do { caracter = getchar(); } while (caracter != '\n' && caracter != EOF);` consume y descarta el excedente para que no contamine la siguiente lectura del menú.
2. **Detección de desbordamiento numérico (`errno == ERANGE`)**:
   Al convertir la cadena con `strtol` o `strtod`, se verifica `errno == ERANGE`, los límites `INT_MIN` / `INT_MAX` y la macro `isfinite(resultado)` para descartar valores infinitos o `NaN` (*Not a Number*).
3. **Rechazo de entradas mixtas (`solo_espacios`)**:
   Si un usuario escribe `"12abc"`, `scanf` aceptaría el `12` y dejaría `"abc"` en el búfer. Nuestro validador comprueba mediante el puntero `fin` y `solo_espacios(fin)` que después del número solo existan espacios en blanco antes del fin de línea.

---

## Declaración de Uso de Inteligencia Artificial (IA)

En cumplimiento con las buenas prácticas de integridad académica, el grupo declara que se utilizó asistencia de **Inteligencia Artificial generativa** como herramienta de consulta, tutoría y apoyo técnico en áreas puntuales que excedían el nivel inicial de programación del equipo:

1. **Revisión y Validación del Modelo Físico**:
   * Verificación del planteamiento analítico del potencial gravitatorio específico combinado Tierra-Luna ($u(r) = -GM_T/r - GM_L/(D-r)$) y de la deducción algebraica del punto de equilibrio gravitatorio estático.
   * Contraste numérico de los resultados del programa frente a los valores teóricos del Escenario 3 (confirmando por qué con $v_{\text{TLI}} = 10\,900\text{ m/s}$ existe un leve déficit energético de $\approx 33.48\text{ kJ/kg}$ en el modelo 1D para alcanzar el punto de equilibrio a $345\,672.69\text{ km}$).
2. **Depuración Avanzada de Código y Manejo de `stdin` (`validaciones.c`)**:
   * Dado que la validación segura de flujos de entrada en C mediante punteros dobles (`strtol`/`strtod`), el control de banderas de error del sistema (`errno.h`, `limits.h`) y el vaciado manual del búfer de entrada (`stdin`) representan un tópico de alta complejidad para el nivel actual del grupo, se recurrió a la IA para estructurar y depurar el módulo `validaciones.c` y resolver errores de compilación y sincronización en Git.
   * Todo el código sugerido fue analizado, comentado y adaptado por los integrantes del grupo para asegurar su total comprensión tanto en la defensa oral como en la integración con `main.c` y `artemis.c`.

---

## Respuestas Analíticas a la Consigna

### a) 1.ª Ley de Newton (Inercia) y desaceleración gravitatoria

Una vez que la etapa ICPS se desacopla y se apagan los motores, la nave Orion avanza por inercia sin empuje propulsivo. De acuerdo con la **Primera Ley de Newton**, la velocidad solo permanecería constante si la fuerza neta externa fuera nula. Sin embargo, sobre la nave actúa la fuerza gravitatoria resultante de la Tierra y la Luna.

Al iniciar el tramo inercial en LEO ($r_i = R_T + h_{\text{LEO}} = 6\,563\,000\text{ m}$), la aceleración neta es:

$$a_{\text{neta}} = -\frac{G M_T}{r_i^2} + \frac{G M_L}{(D - r_i)^2} \approx -9.252855\text{ m/s}^2$$

El signo negativo demuestra que la atracción terrestre frena progresivamente a la nave a medida que se aleja de la Tierra.

### b) Energía cinética inicial tras la maniobra TLI

Para una masa $m = 26\,000\text{ kg}$ y una velocidad inicial $v_i = 10\,900\text{ m/s}$:

$$K_i = \frac{1}{2} m v_i^2 = 0.5 \times 26\,000\text{ kg} \times (10\,900\text{ m/s})^2 = 1.544530 \times 10^{12}\text{ J} \approx 1.545\text{ TJ}$$

Energía cinética específica (por unidad de masa):

$$k_i = \frac{1}{2} v_i^2 = 59\,405\,000\text{ J/kg}$$

### c) Punto de equilibrio gravitatorio y trabajo adicional mínimo ($W_{\text{ext}} / m$)

El punto donde se igualan en magnitud las fuerzas gravitatorias de la Tierra y la Luna se ubica a:

$$r_{\text{eq}} = \frac{D}{1 + \sqrt{M_L / M_T}} \approx 345\,672.69\text{ km desde el centro terrestre}$$

El cambio de potencial gravitatorio específico desde LEO ($r_i = 6\,563\text{ km}$) hasta $r_{\text{eq}}$ es:

$$\Delta u = u(r_{\text{eq}}) - u(r_i) \approx 59\,438\,484.33\text{ J/kg}$$

La velocidad mínima idealizada de salida para alcanzar ese punto con velocidad final nula es:

$$v_{\text{min}} = \sqrt{2 \Delta u} \approx 10\,903.07\text{ m/s}$$

* **Sin ajuste adicional ($v = 10\,900\text{ m/s}$)**: Como $k_i < \Delta u$, la energía cinética inicial no alcanza por un pequeño margen y se requiere un **trabajo específico adicional mínimo (`deficit_energetico_j_kg`)** de:
  $$w_{\text{ext}} = \Delta u - k_i \approx 33\,484.33\text{ J/kg}$$
* **Con ajuste pequeño ($+10\text{ m/s} \implies v = 10\,910\text{ m/s}$)**: La nave supera el umbral energético ($w_{\text{ext}} = 0.00\text{ J/kg}$) y cruza el punto de equilibrio.

### d) Justificación energética del consumo de combustible (Ida vs. Regreso)

La masa de la Tierra es $\approx 81.3$ veces mayor que la de la Luna ($M_T \gg M_L$), lo que crea una marcada asimetría entre ambos pozos de potencial gravitatorio:

1. **Ida (Tierra $\to$ Luna)**: Salir desde LEO hasta el punto de equilibrio exige vencer una barrera de potencial de $\Delta u \approx 59.44\text{ MJ/kg}$ (además del enorme trabajo previo para subir desde la superficie terrestre hasta LEO).
2. **Regreso (Luna $\to$ Tierra)**: Vencer la gravedad lunar desde las cercanías de la Luna hasta el punto de equilibrio requiere apenas $\approx 1.6\text{ MJ/kg}$. Adicionalmente, por la **ecuación del cohete de Tsiolkovski**, al despegar de la Tierra es necesario acelerar toda la masa de combustible que se utilizará en las maniobras posteriores, multiplicando exponencialmente el propelente requerido al inicio de la misión.

---

## Compilación y Ejecución

## 1. Requisitos previos

Para trabajar en **Windows** con Visual Studio Code se necesita:

- **Visual Studio Code** (recomendado para editar el código; no compila por sí solo).
- **MSYS2 UCRT64**, con **GCC** (compilador de C) y **MinGW Make** (`mingw32-make`).
- La carpeta `C:\msys64\ucrt64\bin` incorporada al `Path` de Windows, si se desea compilar desde la terminal de VS Code (PowerShell). La ruta puede variar según dónde se haya instalado MSYS2.

Desde la terminal **MSYS2 UCRT64**, las herramientas se pueden instalar con:

```bash
pacman -Syu
pacman -S mingw-w64-ucrt-x86_64-toolchain
```

Si la actualización solicita cerrar la terminal, abrila nuevamente y completá `pacman -Syu` antes de instalar el grupo de herramientas.

Para verificar la instalación, ejecutá:

```bash
gcc --version
mingw32-make --version
```

## 2. ¿Qué hace el Makefile?

El `Makefile` **automatiza la compilación**: evita tener que escribir manualmente el comando completo de GCC cada vez.

En este proyecto:

- Utiliza **GCC** con el estándar **C11**, advertencias del compilador y optimización `-O2`.
- Compila conjuntamente `main.c`, `artemis.c` y `validaciones.c`.
- Considera `artemis.h` y `validaciones.h` como dependencias: si cambia alguno de estos archivos, vuelve a generar el programa al ejecutar Make.
- En Windows produce **`artemis.exe`**. En sistemas no Windows produce **`artemis`**.
- Incluye la biblioteca matemática de C mediante **`-lm`**, necesaria para algunas funciones matemáticas usadas en la simulación.
- Ofrece el objetivo **`clean`** para eliminar el ejecutable generado.

No es necesario modificar el `Makefile` para compilar la versión entregada.

## 3. Compilar ARTEMIS

1. Abrí en VS Code la **carpeta raíz del repositorio**, donde se encuentra el archivo `Makefile`.
2. Abrí una terminal en esa carpeta.
3. Ejecutá:

```bash
mingw32-make
```

Si el proceso termina sin errores, se habrá generado **`artemis.exe`** en la misma carpeta. Ese archivo es el programa compilado y se puede ejecutar sin volver a compilar, siempre que se conserven los componentes necesarios del entorno de ejecución.

## 4. Ejecutar el programa

Desde **PowerShell** de VS Code:

```powershell
.\artemis.exe
```

Desde **MSYS2 UCRT64**:

```bash
./artemis.exe
```

Se abrirá el menú de ARTEMIS en la terminal para comenzar la simulación.

## 5. Limpiar y volver a compilar (opcional)

Para borrar el ejecutable creado:

```bash
mingw32-make clean
```

Para generarlo nuevamente:

```bash
mingw32-make
```

**Nota:** en Linux u otros entornos compatibles, con GCC y Make instalados, se utiliza `make` para compilar y `./artemis` para ejecutar. El `Makefile` ya contempla ese nombre de ejecutable.

---

## Límites del Modelo

* El programa implementa un **modelo educativo unidimensional (1D)** con Tierra y Luna fijas como masas puntuales, concebido para analizar el principio de conservación de la energía mecánica y la 1.ª Ley de Newton.
* Superar el umbral energético del punto de equilibrio 1D no demuestra por sí solo una inserción lunar real ni reproduce la geometría orbital tridimensional de una trayectoria de retorno libre (*Free Return Trajectory*).