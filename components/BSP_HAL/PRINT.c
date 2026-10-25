/* ===================================================
 * PRINT.c — Todas las impresiones a consola del sistema
 * Elaborado por:
 *   Cardona Valenzuela Ana Paola 22061031
 *   Pérez Dyck Emiliano 22061061
 * =================================================== */
#include "BSP.h"

// -------------------------------------------------------
// PANTALLA DE BIENVENIDA
// -------------------------------------------------------
void Print_Console_Guide(void) {
    printf("\n==================================================\n");
    printf("   LABORATORIO DE MACHINE LEARNING - ESP32\n");
    printf("   Perceptron simple con LMS - ESP32\n");
    printf("   Instituto Tecnologico de Chihuahua\n");
    printf("==================================================\n");
    printf("Sistema APAGADO. Presione PBR para encender.\n");
}

// -------------------------------------------------------
// MENSAJES DE ESTADO DEL SISTEMA
// -------------------------------------------------------
void Print_Sistema_Encendido(void) {
    printf("\n>> Sistema ENCENDIDO.\n");
}

void Print_Sistema_Estado(bool estado) {
    printf("\n>> ESTADO: %s\n", estado ? "RUN" : "STOP");
}

void Print_ADC_Listo(void) {
    printf("\n=========================================\n");
    printf("  INICIO DE LECTURA \n");
    printf("  (Set de Prueba)\n");
    printf("  Presione PBR para detener el sistema.\n");
    printf("=========================================\n\n");
}

// -------------------------------------------------------
// MENUS DE CONFIGURACION
// -------------------------------------------------------
void Print_Pedir_Dimensiones(void) {
    printf("\n>> Ingrese el numero de dimensiones (2 a 5): ");
    fflush(stdout);
}

void Print_Error_Dimensiones(int val) {
    printf("\nError: %d no es valido. Ingrese de 2 a 5: ", val);
    fflush(stdout);
}

void Print_Pedir_Salida(void) {
    printf("\n>> Seleccione tipo de salida:\n");
    printf("   1. AND\n");
    printf("   2. OR\n");
    printf("   3. Personalizada\n");
    printf("Eleccion: ");
    fflush(stdout);
}

void Print_Error_Salida(void) {
    printf("\nError: Elija 1, 2 o 3: ");
    fflush(stdout);
}

// -------------------------------------------------------
// ENTRENAMIENTO — TABLA DE VERDAD
// -------------------------------------------------------
void Print_TruthTable(int **x, int *y, int rows, int dims) {
    printf("\n--- TABLA DE VERDAD (Set de Entrenamiento) ---\n");
    for (int j = 0; j < dims; j++) printf("[X%d] ", j + 1);
    printf("| [Salida Y]\n");
    printf("----------------------------------------------\n");
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < dims; j++) printf(" %d   ", x[i][j]);
        printf("|     %d\n", y[i]);
    }
    printf("----------------------------------------------\n");
}

// -------------------------------------------------------
// ENTRENAMIENTO — FILA PERSONALIZADA (opcion 3)
// -------------------------------------------------------
void Print_Fila_Personalizada(int fila, int *x_row, int dims) {
    printf("\nFila %d: ", fila);
    for (int j = 0; j < dims; j++) printf("%d ", x_row[j]);
    printf("| Ingrese salida deseada (0 o 1): ");
    fflush(stdout);
}

void Print_Error_Salida_Personalizada(void) {
    printf("Error: Solo 0 o 1.\n");
}

// -------------------------------------------------------
// ENTRENAMIENTO — PROGRESO
// -------------------------------------------------------
void Print_Entrenando(void) {
    printf("\nEntrenando perceptron...\n");
}

void Print_Entrenamiento_Finalizado(int epochs) {
    printf("Entrenamiento finalizado en %d epocas.\n", epochs);
}

// -------------------------------------------------------
// PESOS DEL PERCEPTRON
// Llamar despues del entrenamiento y despues de la validacion
// w[0..dim-1] = pesos de entradas, w[dim] = bias
// -------------------------------------------------------
void Print_Pesos(float *w, int dim) {
    printf("\n--- PESOS DEL PERCEPTRON ---\n");
    for (int i = 0; i < dim; i++) {
        printf("  w[%d] (X%d)  = %8.5f\n", i, i + 1, w[i]);
    }
    printf("  w[%d] (bias) = %8.5f\n", dim, w[dim]);
    printf("----------------------------\n");
}

// -------------------------------------------------------
// ENTRENAMIENTO — REPORTE DE VALIDACION INTERNA
// -------------------------------------------------------
void Print_ML_Results(int **x, int *expected, int *obtained, int rows, int dims) {
    printf("\n--- REPORTE DE VALIDACION INTERNA ---\n");
    for (int j = 0; j < dims; j++) printf("[X%d] ", j + 1);
    printf("| [Esperado] | [Obtenido] | [Estado]\n");
    printf("------------------------------------------------------------\n");

    int exitos = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < dims; j++) printf(" %d   ", x[i][j]);
        bool correcto = (expected[i] == obtained[i]);
        if (correcto) exitos++;
        printf("|      %d     |      %d     | %s\n",
               expected[i], obtained[i], correcto ? "OK" : "ERROR");
    }

    printf("------------------------------------------------------------\n");
    printf("RESULTADO: %d/%d aciertos. ", exitos, rows);
    if (exitos == rows) printf("Modelo Convergente!\n");
    else                printf("Modelo No Convergente.\n");
}

// ----------------------------------------------------------------------
// VALIDACION CON DATOS DEL POTENCIOMETRO
// Muestra el valor crudo del ADC, su codificacion binaria
// por umbral (THRESHOLD_ADC) y la salida del perceptron.
// ----------------------------------------------------------------------
void Print_Validacion(int *values, int *bin_values, int n, int ml_out) {
    printf("[ ");
    for (int i = 0; i < n; i++) {
        printf("X%d:%4d->%d  ", i + 1, values[i], bin_values[i]);
    }
    printf("]  Salida: %d  LED_ML: %s\n",
           ml_out, ml_out ? "ON" : "OFF");
}