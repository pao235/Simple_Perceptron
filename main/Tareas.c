/* ================================================
 * INSTITUTO TECNOLÓGICO DE CHIHUAHUA
 * Materia: Inteligencia Artificial
 * Tarea 6: Laboratorio de Machine Learning
 * Tareas.c — Tareas RTOS: Lectura, ML y Consola
 * Elaborado por:
*  	Cardona Valenzuela Ana Paola 22061031
*   Pérez Dyck Emiliano 22061061
 * ================================================ */
#include "BSP.h"

// --- TAREA 1: LECTURA DE SENSORES ---
void lectura(void *pvParameters) {
    while (1) {
        if (rmode) { // Solo opera si el sistema general está en "Run"
            for (int i = 0; i < g_inputs.n_actual; i++) {
                int raw = ADC_Read(ADC_CHANNELS[i]); // Obtiene lectura analógica
                g_inputs.values[i] = raw; // Guarda el valor crudo en la estructura global
                
                // Aplica histéresis para discretizar la entrada
                if (raw > THRESH_HIGH) {
                    g_inputs.bin_values[i] = 1; // Sube por encima de 2148 -> Señal en Alto
                } else if (raw < THRESH_LOW) {
                    g_inputs.bin_values[i] = 0; // Baja de 1948 -> Señal en Bajo
                }
                // (Si está entre 1948 y 2148, mantiene el valor anterior)
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20)); // Cede control al RTOS por 20 milisegundos
    }
}

// --- TAREA 2: INFERENCIA DEL PERCEPTRON ---
void ML_Task(void *pvParameters) {
    while (1) {
        if (rmode) {
            // Pasa las lecturas discretizadas a la red neuronal
            g_outputs.ml_out = ML_Inference((int*)g_inputs.bin_values,
                                             g_inputs.n_actual);
            // Refleja la decisión de la IA encendiendo o apagando un LED
            gpio_set_level(LED_ML, g_outputs.ml_out);
        } else {
            gpio_set_level(LED_ML, 0); // Apaga el LED si el sistema está detenido
        }
        vTaskDelay(pdMS_TO_TICKS(20)); // Cede control al RTOS por 20 milisegundos
    }
}

// --- TAREA 3: CONSOLA (SET DE VALIDACIÓN EN TIEMPO REAL) ---
void Consola_Task(void *pvParameters) {
    while (1) {
        if (rmode) {
            Print_Validacion(
                (int*)g_inputs.values,
                (int*)g_inputs.bin_values,
                g_inputs.n_actual,
                g_outputs.ml_out
            );
        }
        vTaskDelay(pdMS_TO_TICKS(200)); // Cede control al RTOS por 200 milisegundos 
    }
}