/* ==================================================================
 * INSTITUTO TECNOLÓGICO DE CHIHUAHUA
 * Materia: Inteligencia Artificial
 * Tarea 6: Laboratorio de Machine Learning
 * GPIO.c — Manejo de entradas y salidas digitales
 * Elaborado por: 
 * 	Cardona Valenzuela Ana Paola 22061031
 * 	Pérez Dyck Emiliano 22061061
 * ==================================================================*/
#include "BSP.h"

// Arreglo de configuración que define los pines del sistema y su comportamiento
config_gpio_t gpio_list[] = {
    // Pin, Modo, Pull-up, Pull-down, Tipo de interrupción
    {LED_STATE, GPIO_MODE_OUTPUT, false, false, GPIO_INTR_DISABLE}, // LED de estado general
    {LED_ML, GPIO_MODE_OUTPUT, false, false, GPIO_INTR_DISABLE}, // LED de salida del perceptrón
    {PBR, GPIO_MODE_INPUT,  true,  false, GPIO_INTR_NEGEDGE}  // Botón con pull-up, interrupción en flanco de bajada
};

int64_t  last_isr_time = 0; // Almacena el tiempo de la última interrupción válida (para debounce)
volatile SystemInputs g_inputs = { .n_actual = 2 }; // Estructura con las lecturas de entrada (inicia en 2 dimensiones)
SystemOutputs_t g_outputs  = {0}; // Estructura para las salidas del sistema

// Rutina de Servicio de Interrupción (ISR) para el botón PBR
static void IRAM_ATTR debounce(void* arg) {
    int64_t now = esp_timer_get_time(); // Obtiene el tiempo actual en microsegundos
    // Antirrebote 
    if ((now - last_isr_time) > 250000) {
        // Verifica si el botón realmente está presionado 
        if (gpio_get_level(PBR) == 0) {
            rmode = !rmode; // Alterna el estado del sistema (Run/Stop)
            last_isr_time = now; // Actualiza el tiempo de la última interrupción válida
        }
    }
}

// Inicializa todos los pines definidos en gpio_list
void gpio_init_all(void) {
    int n = sizeof(gpio_list) / sizeof(gpio_list[0]); // Calcula la cantidad de pines a configurar
    for (int i = 0; i < n; i++) gpio_init(&gpio_list[i]); // Aplica la configuración a cada pin
    
    gpio_install_isr_service(0); // Instala el servicio global de interrupciones GPIO
    gpio_isr_handler_add(PBR, debounce, NULL); // Asigna la función debounce a la interrupción del botón PBR
}

// Configura un pin individual utilizando las librerías nativas de ESP-IDF
void gpio_init(const config_gpio_t *cfg) {
    gpio_config_t conf = {
        .pin_bit_mask = (1ULL << cfg->pin), // Selecciona el pin usando un desplazamiento de bits
        .mode         = cfg->mode, // Asigna modo (entrada/salida)
        .pull_up_en   = cfg->pullup   ? GPIO_PULLUP_ENABLE   : GPIO_PULLUP_DISABLE,
        .pull_down_en = cfg->pulldown ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE,
        .intr_type    = cfg->intr_type  // Configura el tipo de interrupción
    };
    gpio_config(&conf); // Aplica la configuración
}

// Función para leer de forma segura el ADC
int ADC_Read(adc1_channel_t canal) {
    int raw = adc1_get_raw(canal); // Obtiene el valor crudo del canal (0-4095)
    return (raw >= 0) ? raw : -1;  // Retorna el valor o -1 si hubo un error de lectura
}