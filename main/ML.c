/* =================================================================================
 * INSTITUTO TECNOLÓGICO DE CHIHUAHUA
 * Materia: Inteligencia Artificial
 * Tarea 6: Laboratorio de Machine Learning
 * ML.c — Capa de Machine Learning (Perceptron Simple / LMS)
 * Elaborado por: 
 * 	Cardona Valenzuela Ana Paola 22061031
 * 	Pérez Dyck Emiliano 22061061
 * ================================================================================= */

#include "BSP.h"

float *w = NULL;// Puntero para almacenar los pesos sinápticos
int dim_global = 0; // Dimensión (número de entradas) global

// Función de activación Escalon Unitario: retorna 1 si la suma ponderada >= 0, sino 0
int activation(float n) {
    return (n >= 0) ? 1 : 0;
}

// Inicializa, genera el set de datos y entrena el perceptrón
void ML_Init(int dim, int sel) {
    dim_global = dim;
    int num_rows = (int)pow(2, dim); // Calcula el total de combinaciones lógicas (2^n)

    // ASIGNACIÓN DE MEMORIA DINÁMICA para matriz de entradas X
    int **x = (int **)malloc(num_rows * sizeof(int *));
    for (int i = 0; i < num_rows; i++)
        x[i] = (int *)malloc(dim * sizeof(int));

    // Memoria para salidas deseadas (y) y salidas obtenidas en validación (obtained)
    int *y = (int *)malloc(num_rows * sizeof(int));
    int *obtained = (int *)malloc(num_rows * sizeof(int));

    // Limpia y reserva memoria para los pesos (+1 para incluir el sesgo)
    if (w != NULL) free(w);
    w = (float *)malloc((dim + 1) * sizeof(float));

    // GENERAR TABLA DE VERDAD
    for (int i = 0; i < num_rows; i++) {
        int res_and = 1, res_or = 0;
        for (int j = 0; j < dim; j++) {
            // Genera los bits lógicos (0s y 1s) para cada columna utilizando división entera y módulo
            x[i][j] = (i / (int)pow(2, dim - 1 - j)) % 2;
            
            // Lógica precalculada para AND y OR
            if (x[i][j] == 0) res_and = 0; // En una AND, un solo 0 hace toda la salida 0
            if (x[i][j] == 1) res_or  = 1; // En una OR, un solo 1 hace toda la salida 1
        }

        // Asigna la salida esperada (y) basándose en la selección del usuario
        if (sel == 1) {
            y[i] = res_and;
        } else if (sel == 2) {
            y[i] = res_or;
	    } else {
            // Opcion 3: entrada personalizada por consola
            Print_Fila_Personalizada(i, x[i], dim);
            while (1) {
                // Leemos un solo caracter del puerto serie
                int c = fgetc(stdin);
                
                // Si el caracter es '0' o '1' (como texto)
                if (c == '0' || c == '1') {
                    y[i] = c - '0'; // Convertimos el caracter ASCII a número entero
                    printf("%d\n", y[i]); 
                    break; // Lectura exitosa, salimos del ciclo
                } 
                // Si lee algo que no es vacío, ni salto de línea, ni espacios
                else if (c != EOF && c != '\n' && c != '\r' && c != ' ') {
                    Print_Error_Salida_Personalizada();
                    Print_Fila_Personalizada(i, x[i], dim); // Repetimos la solicitud
                }
                
                vTaskDelay(pdMS_TO_TICKS(10)); 
            }
         }
    }

    Print_TruthTable(x, y, num_rows, dim);

    // PESOS INICIALES ALEATORIOS
    srand((unsigned int)esp_timer_get_time()); // Semilla basada en el timer de hardware
    for (int i = 0; i <= dim; i++)
        // Genera pesos entre -1.0 y 1.0
        w[i] = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f; 

    // ENTRENAMIENTO (Algoritmo LMS - Least Mean Squares)
    Print_Entrenando();

    int epoch = 0, error_exists = 1;
    // Bucle de aprendizaje: itera hasta que no haya errores o se alcance el máximo de épocas
    while (error_exists && epoch < MAX_EPOCHS) {
        error_exists = 0; // Asume que no hay error al inicio de la época
        for (int i = 0; i < num_rows; i++) {
            float r = w[dim]; // Inicia el acumulador con el valor del sesgo
            
            // Producto punto: Sumatoria(Pesos * Entradas)
            for (int j = 0; j < dim; j++) r += w[j] * x[i][j];

            int out   = activation(r); // Pasa por la función escalón
            int error = y[i] - out;    // Calcula el error (Deseado - Obtenido)

            // Si hay error, ajusta los pesos
            if (error != 0) {
                for (int j = 0; j < dim; j++)
                    // Regla Delta: W_nuevo = W_actual + (Tasa * Error * Entrada)
                    w[j] += LEARNING_RATE * error * x[i][j]; 
                
                // Ajusta también el sesgo (asume entrada constante de 1)
                w[dim] += LEARNING_RATE * error; 
                error_exists = 1; // Bandera para obligar a otra época de revisión
            }
        }
        epoch++;
    }

    Print_Entrenamiento_Finalizado(epoch);
    Print_Pesos(w, dim);

    // VALIDACIÓN INTERNA
    // Repasa todo el dataset con los pesos finales para confirmar que todo funciona
    for (int i = 0; i < num_rows; i++) {
        float r = w[dim];
        for (int j = 0; j < dim; j++) r += w[j] * x[i][j];
        obtained[i] = activation(r);
    }

    Print_ML_Results(x, y, obtained, num_rows, dim);
    Print_Pesos(w, dim);

    // LIBERACIÓN DE MEMORIA TEMPORAL 
    // Libera la tabla de verdad, pero mantiene el arreglo global 'w' para usarlo en ejecución
    for (int i = 0; i < num_rows; i++) free(x[i]);
    free(x);
    free(y);
    free(obtained);
}

// Función que realiza la clasificación en tiempo real usando los pesos entrenados
int ML_Inference(int *x_inputs, int n) {
    if (w == NULL) return 0; // Proteccion si los pesos no existen
    float r = w[n]; // Inicia con el sesgo guardado en la ultima posición del arreglo w
    for (int i = 0; i < n; i++) r += w[i] * x_inputs[i]; // Sumatoria
    return activation(r); // Retorna el veredicto (0 o 1)
}