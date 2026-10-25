/* ========================================================
 * INSTITUTO TECNOLÓGICO DE CHIHUAHUA
 * Materia: Inteligencia Artificial
 * Tarea 6: Laboratorio de Machine Learning
 * Elaborado por:
 *   Cardona Valenzuela Ana Paola 22061031
 *   Pérez Dyck Emiliano 22061061
 * ======================================================== */
 
#include "BSP.h"
 
 /* rmode: bandera global que indica si el sistema está en RUN (true) o STOP (false).
 * Es volatile porque la modifica la ISR del botón PBR desde otro contexto de ejecución. */
volatile bool rmode = false;
 
/* reconfig_pending: señal auxiliar para indicar que se requiere una reconfiguración
 * del perceptron en el próximo ciclo RUN. */
volatile bool reconfig_pending = false;
 
/* Estructura que almacena las características de calibración del ADC.
 * Se llena en ADC_Init() y se usa para convertir valores a mV. */
esp_adc_cal_characteristics_t g_adc_chars;

/* Handles de las tres tareas RTOS. Se guardan para poder suspenderlas y
 * reanudarlas durante la reconfiguración sin destruir y recrear las tareas. */
TaskHandle_t h_lectura = NULL;
TaskHandle_t h_ml      = NULL;
TaskHandle_t h_consola = NULL;
 
 
void app_main(void)
{
    /* --- Inicialización de hardware ---
     * Se configuran todos los GPIOs (entradas, salidas e interrupción del PBR).
     * Los LEDs arrancan apagados y se imprime la pantalla de bienvenida. */
    gpio_init_all();
    gpio_set_level(LED_STATE, 0);
    gpio_set_level(LED_ML, 0);
    Print_Console_Guide();
 
    /* --- Espera de encendido ---
     * El sistema queda bloqueado hasta que el usuario presione PBR.
     * La ISR del botón pone rmode = true, lo que rompe este ciclo.
     * Se cede la CPU cada 50 ms para no desperdiciar ciclos. */
    while (!rmode) vTaskDelay(pdMS_TO_TICKS(50));
    gpio_set_level(LED_STATE, 1);   // LED_STATE encendido = sistema activo
    Print_Sistema_Encendido();
 
    /* --- Configuración y entrenamiento inicial ---
     * El usuario elige el número de entradas y el tipo de función lógica.
     * Con esos parámetros se genera la tabla de verdad y se entrena el perceptrón. */
    run_config_and_train();
 
    /* --- Inicialización del ADC ---
     * Se configuran los canales ADC según el número de dimensiones elegido
     * y se cargan los parámetros de calibración en g_adc_chars. */
    ADC_Init(&g_adc_chars);
    Print_ADC_Listo();
 
 
     /* Se crean las tres tareas del sistema guardando sus handles:
     *   - lectura:lee y binariza los canales ADC (prioridad 3)
     *   - ML_Task: ejecuta la inferencia del perceptrón (prioridad 3)
     *   - Consola_Task: imprime el estado en el puerto serie (prioridad 1) */
    xTaskCreate(lectura,      "Lectura",  4096, NULL, 3, &h_lectura);
    xTaskCreate(ML_Task,      "ML_Layer", 4096, NULL, 3, &h_ml);
    xTaskCreate(Consola_Task, "Consola",  4096, NULL, 1, &h_consola);
 
    /* --- LOOP PRINCIPAL ---
     * Monitorea rmode en cada iteración para:
     *   1. Reflejar el estado del sistema en LED_STATE.
     *   2. Detectar la transición STOP -> RUN y lanzar la reconfiguración. */
    bool last_rmode = true;
    while (1) {
        bool cur = rmode;
 
        // Sincronizar LED_STATE con el estado actual del sistema
        gpio_set_level(LED_STATE, cur ? 1 : 0);
 
        // Actuar solo cuando el estado cambia para no ejecutar código innecesario
        if (last_rmode != cur) {
            Print_Sistema_Estado(cur);  // Imprime ">> ESTADO: RUN" o ">> ESTADO: STOP"
 
            if (!last_rmode && cur) {
                /* Transición STOP -> RUN detectada:
                 * Las tareas RTOS deben detenerse antes de la reconfiguración
                 * porque Consola_Task y lectura usan getchar()/printf() que
                 * colisionarían con el menú de configuración en el puerto serie. */
 
                // Suspender las tareas para liberar el puerto serie
                if (h_lectura)  vTaskSuspend(h_lectura);
                if (h_ml)       vTaskSuspend(h_ml);
                if (h_consola)  vTaskSuspend(h_consola);
 
                // Reconfigurar parámetros y reentrenar el perceptrón
                run_config_and_train();
                ADC_Init(&g_adc_chars);
                Print_ADC_Listo();
 
                // Reanudar las tareas con el nuevo modelo entrenado
                if (h_lectura)  vTaskResume(h_lectura);
                if (h_ml)       vTaskResume(h_ml);
                if (h_consola)  vTaskResume(h_consola);
            }
 
            last_rmode = cur;
        }
 
        // Ceder CPU al scheduler durante LOOP_DELAY_MS antes de la siguiente revisión
        vTaskDelay(pdMS_TO_TICKS(LOOP_DELAY_MS));
    }
}
 
/* leer_digito — Lee un carácter '0'-'9' del puerto serie y lo devuelve como entero.
 * Descarta cualquier otro carácter y cede la CPU 50 ms entre intentos
 * para no bloquear el scheduler mientras espera la entrada del usuario. */
int leer_digito(void) {
    int c;
    while (1) {
        c = getchar();
        if (c >= '0' && c <= '9') return c - '0';
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
 
/* run_config_and_train — Solicita al usuario el número de dimensiones (2-5)
 * y el tipo de función de salida (AND / OR / Personalizada), valida ambas
 * entradas en bucle y llama a ML_Init() para generar la tabla de verdad,
 * entrenar el perceptrón e imprimir los resultados de validación interna. */
void run_config_and_train(void) {
    // Solicitar y validar el número de dimensiones
    int dim = 0;
    Print_Pedir_Dimensiones();
    while (1) {
        dim = leer_digito();
        if (dim >= MIN_INPUTS && dim <= MAX_INPUTS) {
            g_inputs.n_actual = dim;    // Actualizar número de canales activos
            break;
        }
        Print_Error_Dimensiones(dim);   // Valor fuera de rango: pedir de nuevo
    }
    
    // Solicitar y validar el tipo de función lógica de salida
    int sel = 0;
    Print_Pedir_Salida();
    while (1) {
        sel = leer_digito();
        if (sel >= 1 && sel <= 3) break;
        Print_Error_Salida(); // Opción inválida: pedir de nuevo
    }
 
    // Generar tabla de verdad, entrenar perceptrón y mostrar reporte de validación
    ML_Init(g_inputs.n_actual, sel);
}