/* ==================================================================
 * INSTITUTO TECNOLÓGICO DE CHIHUAHUA
 * Materia: Inteligencia Artificial
 * Tarea 6: Laboratorio de Machine Learning
 * BSP - Board Support Package 
 * Elaborado por: 
 * 	Cardona Valenzuela Ana Paola 22061031
 * 	Emiliano Pérez Dyck 22061061
 * ==================================================================*/

#ifndef BSP_H
#define BSP_H

// Inclusión de librerías estándar de C, RTOS y ESP-IDF necesarias
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "hal/gpio_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

/* --- CONFIGURACIÓN DE HARDWARE --- */
#define MIN_INPUTS      2 // Mínimo de dimensiones (entradas) para el modelo
#define MAX_INPUTS      5 // Máximo de dimensiones (entradas) para el modelo

// Umbral central y banda de histéresis para binarizar entradas analógicas
// (±HYST_ADC alrededor de 2048 para evitar falsos cambios por ruido eléctrico)
#define THRESHOLD_ADC   2048
#define HYST_ADC        200
#define THRESH_HIGH     (THRESHOLD_ADC + HYST_ADC)   // Sube a 2148 -> Se considera "1" lógico
#define THRESH_LOW      (THRESHOLD_ADC - HYST_ADC)   // Baja a 1948 -> Se considera "0" lógico

// Asignación de pines del ESP32
#define PBR        GPIO_NUM_4
#define LED_STATE  GPIO_NUM_18
#define LED_ML     GPIO_NUM_19

// Arreglo con los canales del ADC1 asociados a los potenciómetros/sensores
static const adc1_channel_t ADC_CHANNELS[MAX_INPUTS] = {
    ADC_CHANNEL_0, ADC_CHANNEL_3, ADC_CHANNEL_6,
    ADC_CHANNEL_7, ADC_CHANNEL_4
};

/* --- ESTRUCTURAS --- */
// Estructura para facilitar la inicialización de puertos GPIO
typedef struct {
    gpio_num_t       pin;
    gpio_mode_t      mode;
    bool             pullup;
    bool             pulldown;
    gpio_int_type_t  intr_type;
} config_gpio_t;

// Estructura para agrupar los datos de entrada del sistema
typedef struct {
    int n_actual;               // Cantidad de entradas activas en uso
    int values[MAX_INPUTS];     // Valores analógicos crudos leídos del ADC
    int bin_values[MAX_INPUTS]; // Valores discretizados (0 o 1) aplicando la histéresis
} SystemInputs;

// Estructura para la salida del sistema
typedef struct {
    int ml_out; // Veredicto final de la red neuronal (0 o 1)
} SystemOutputs_t;

/* --- VARIABLES GLOBALES --- */
extern volatile bool  rmode; // Bandera general de ejecución (Run/Stop)
extern volatile bool  reconfig_pending; // Bandera para reiniciar/reconfigurar modelo
extern volatile SystemInputs  g_inputs;  // Estructura de entrada
extern SystemOutputs_t  g_outputs; // Estructura de salidas
extern esp_adc_cal_characteristics_t g_adc_chars; // Datos de calibración del ADC

// Referencias (handles) para controlar las tareas de FreeRTOS
extern TaskHandle_t h_lectura;
extern TaskHandle_t h_ml;
extern TaskHandle_t h_consola;

/* --- PROTOTIPOS GPIO (GPIO.c) --- */
void gpio_init_all(void);
void gpio_init(const config_gpio_t *cfg);
int  ADC_Read(adc1_channel_t canal);

/* --- PROTOTIPOS ADC (ADC.c) --- */
void ADC_Init(esp_adc_cal_characteristics_t *adc_chars);

/* --- PROTOTIPOS CONSOLA (PRINT.c) --- */
void Print_Console_Guide(void);
void Print_Sistema_Encendido(void);
void Print_Sistema_Estado(bool estado);
void Print_ADC_Listo(void);
void Print_Pedir_Dimensiones(void);
void Print_Error_Dimensiones(int val);
void Print_Pedir_Salida(void);
void Print_Error_Salida(void);
void Print_TruthTable(int **x, int *y, int rows, int dims);
void Print_Fila_Personalizada(int fila, int *x_row, int dims);
void Print_Error_Salida_Personalizada(void);
void Print_Entrenando(void);
void Print_Entrenamiento_Finalizado(int epochs);
void Print_Pesos(float *w, int dim);
void Print_ML_Results(int **x, int *expected, int *obtained, int rows, int dims);
void Print_Validacion(int *values, int *bin_values, int n, int ml_out);
void run_config_and_train(void);

/* --- PROTOTIPO LECTURA SERIE (main.c) --- */
int leer_digito(void);

/* --- MACHINE LEARNING (ML.c) --- */
// Constantes para el algoritmo LMS (Least Mean Squares)
#define LEARNING_RATE   0.5f // Tasa de aprendizaje (qué tan agresivo ajusta los pesos)
#define MAX_EPOCHS      500  // Límite de iteraciones para evitar ciclos infinitos si el modelo no converge

void ML_Init(int dim, int sel);
int  ML_Inference(int *x_inputs, int n);
int  activation(float n);
extern float *w; // Arreglo de pesos globales

/* --- TAREAS RTOS (Tareas.c) --- */
void lectura(void *pvParameters);
void ML_Task(void *pvParameters);
void Consola_Task(void *pvParameters);

#define LOOP_DELAY_MS 100

#endif // BSP_H