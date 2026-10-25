/* ==================================================================
 * INSTITUTO TECNOLÓGICO DE CHIHUAHUA
 * Materia: Inteligencia Artificial
 * Tarea 6: Laboratorio de Machine Learning
 * ADC.c — Inicialización del módulo ADC
 * Elaborado por: 
 * 	Cardona Valenzuela Ana Paola 22061031
 * 	Pérez Dyck Emiliano 22061061
 * ==================================================================*/
#include "BSP.h"

// Inicializa del ADC y sus características de calibración
void ADC_Init(esp_adc_cal_characteristics_t *adc_chars) {
    // Itera sobre el número de entradas activas configuradas en el sistema
    for (int i = 0; i < g_inputs.n_actual; i++) {
        // Configura la atenuación del canal a 12dB, permitiendo leer voltajes hasta ~3.3V
        adc1_config_channel_atten(ADC_CHANNELS[i], ADC_ATTEN_DB_12);
    }
    // Caracteriza el ADC 1 (resolución de 12 bits, atenuación 12dB, Vref de 1100mV) 
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12,
                             ADC_WIDTH_BIT_12, 1100, adc_chars);
}