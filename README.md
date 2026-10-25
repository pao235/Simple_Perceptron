# Simple Perceptron with LMS on ESP32 · Machine Learning Lab

**2-to-5-input perceptron trained on the microcontroller with the LMS algorithm, layered software architecture (BSP/HAL) and multitasking with FreeRTOS**  
Assignment 6 · Artificial Intelligence — Instituto Tecnológico de Chihuahua (ITCH) · January – June 2026 Semester

## Description

This project implements a **simple perceptron** on an **ESP32** capable of learning, directly on the microcontroller, the logic functions **AND**, **OR**, or a **custom output** defined by the user. Unlike previous assignments, where specific truth tables were found through different techniques, here the algorithm itself finds the weights that satisfy the truth table.

The user configures the system through the serial port (number of inputs and function type). With that data the firmware generates the truth table, trains the perceptron with **LMS** (Least Mean Squares), validates the result, and moves to run mode, where 2 to 5 potentiometers act as analog inputs that are binarized with hysteresis and evaluated in real time. The model's output turns an LED on or off.

The firmware is written in C on **ESP-IDF** and organized in layers, separating the application logic from hardware details.

---

## Main Features

* **Perceptron + LMS on the microcontroller:** full training on the ESP32 with random initial weights in [-1, 1], unit step activation function, and a maximum epoch limit for cases that do not converge.
* **Dynamic console configuration:** the user chooses 2 to 5 dimensions and the function type (AND, OR, or custom, entering the desired output row by row).
* **Truth table generated at runtime:** dynamic memory for the 2^n combinations, freed after training while keeping only the weight vector.
* **Internal validation:** after training, the truth table is traversed again with the final weights and compared against the desired output.
* **Hysteresis binarization:** ADC readings are converted to 0/1 using two thresholds (`THRESH_HIGH` and `THRESH_LOW`) to avoid instability in intermediate readings.
* **Multitasking with FreeRTOS:** three independent tasks (reading, inference and console).
* **RUN/STOP control via button:** interrupt with *debounce*; when going from STOP back to RUN the system reconfigures and retrains without restarting.
* **Layered architecture:** custom BSP and HAL for ADC, GPIO and console.

---

## Hardware and Materials

| Component | Quantity | Function |
| --- | --- | --- |
| **ESP32** | 1 | Microcontroller (ESP32 DevKit) |
| **Potentiometers** | 2 to 5 (depending on chosen dimensions) | Analog inputs of the perceptron (ADC channels) |
| **Push button (PBR)** | 1 | Toggles between RUN and STOP (interrupt with debounce) |
| **Status LED (`LED_STATE`)** | 1 | Shows whether the system is in RUN |
| **Output LED (`LED_ML`)** | 1 | Shows the perceptron's output |

> The exact pin mapping (ADC channels, button and LEDs) is defined in `BSP.h` (`ADC_CHANNELS`, `LED_STATE`, `LED_ML`).

---

## System Architecture

### Software Layers

```
┌───────────────────────────────────────────────────────────┐
│ Application Layer    main.c · ML.c                        │
│   HMI (button, LEDs, console) and perceptron/LMS          │
├───────────────────────────────────────────────────────────┤
│ System Layer         FreeRTOS · Tareas.c                  │
│   Tasks: lectura, ML_Task, Consola_Task                   │
├───────────────────────────────────────────────────────────┤
│ BSP / HAL            BSP.h · ADC.c · PRINT.c              │
│   Constants, GPIO, calibrated ADC, console printing       │
├───────────────────────────────────────────────────────────┤
│ Hardware / Drivers   ESP32 · ESP-IDF                      │
│   Potentiometers, button, LEDs                            │
└───────────────────────────────────────────────────────────┘
```

### Runtime Data Flow

```
┌──────────────┐  ADC   ┌─────────────┐ bin_values ┌─────────────┐  0 / 1   ┌──────────┐
│ Potentiometer│───────>│ lectura     │───────────>│ ML_Task     │─────────>│ LED_ML   │
│ 1 … N (2-5)  │        │ (hysteresis)│            │ (inference) │          └──────────┘
└──────────────┘        └─────┬───────┘            └─────┬───────┘
                              │ values / bin_values      │ ml_out
                              ▼                          ▼
                        ┌───────────────────────────────────┐
                        │ Consola_Task (live validation)    │
                        └───────────────────────────────────┘
```

---

## How It Works

### 1. Startup and Waiting
The system initializes GPIO, turns off the LEDs, shows the console guide and stays in **STOP** until the user presses the button. The button ISR sets the `volatile bool rmode` flag to `true` (RUN) and `LED_STATE` turns on.

### 2. Configuration and Training (`run_config_and_train`)
1. The number of dimensions (2 to 5) is requested and validated.
2. The function type is requested: `1` AND, `2` OR, `3` custom.
3. `ML_Init()` generates the truth table, trains the perceptron and shows the validation results.
4. The ADC is initialized with the corresponding channels and the calibration is loaded into `g_adc_chars`.

### 3. LMS Training (`ML.c`)

For each row of the truth table:

```
r     = w[n] + Σ (w[j] · x[j])        # weighted sum + bias
out   = (r >= 0) ? 1 : 0              # unit step
error = y_desired - out

if error ≠ 0:
    w[j] += LEARNING_RATE · error · x[j]
    w[n] += LEARNING_RATE · error     # bias (constant input 1)
```

The loop repeats over epochs until a full epoch has no errors or `MAX_EPOCHS` is reached (necessary for non-linearly separable functions, such as XOR, where the model does not converge).

### 4. FreeRTOS Tasks (`Tareas.c`)

| Task | Priority | Period | Function |
| --- | --- | --- | --- |
| `lectura` | 3 | 20 ms | Reads the ADC channels and binarizes with hysteresis |
| `ML_Task` | 3 | 20 ms | Runs `ML_Inference()` and updates `LED_ML` |
| `Consola_Task` | 1 | 200 ms | Prints raw values, binary values and the model output |

**Binarization hysteresis:**
* Reading `> THRESH_HIGH` (≈ 2148) → input = 1
* Reading `< THRESH_LOW` (≈ 1948) → input = 0
* Between both thresholds → the previous value is kept

### 5. STOP → RUN Reconfiguration
When the user presses the button again, the main loop detects the transition, **suspends the three tasks** (to free the serial port), runs the configuration and training again, reinitializes the ADC and **resumes** the tasks with the new model.

---

## Console Usage

1. Press the button to switch to RUN.
2. Enter the number of inputs (`2`–`5`).
3. Choose the output function: `1` = AND, `2` = OR, `3` = custom.
4. If option `3` was chosen, enter the desired output (`0` or `1`) for each truth table row shown by the console.
5. Wait for the training report (epochs, weights and validation).
6. Move the potentiometers and watch the console and `LED_ML`.
7. Press the button again to stop and reconfigure with a different function or dimension.

---

## Results

The system was tested with 2 to 5 inputs for the standard AND and OR functions, and with other custom functions, converging without an excessive number of epochs even in dimensions greater than three. As a demonstration of the model's limitations, the two-input **XOR** function, which is not linearly separable, does not converge and training ends when the maximum number of epochs is reached.

---

## Project Structure

```
Perceptron-ESP32/
├── main/
│   ├── main.c        # HMI, RUN/STOP control, configuration and retraining
│   ├── ML.c          # Simple perceptron: truth table, LMS training, inference
│   ├── Tareas.c      # RTOS tasks: lectura, ML_Task, Consola_Task
│   ├── BSP.h         # Constants, global variables and prototypes
│   ├── ADC.c         # Calibrated ADC initialization and reading
│   ├── PRINT.c       # Console printing functions
│   └── CMakeLists.txt
├── CMakeLists.txt
└── README.md
```

---

## Software and Build Requirements

| Tool | Recommended Version |
| --- | --- |
| **ESP-IDF** | v4.x / v5.x (uses `esp_adc_cal`) |
| **Board** | ESP32 DevKit |

```bash
# Set the target
idf.py set-target esp32

# Build
idf.py build

# Flash and open serial monitor
idf.py -p /dev/ttyUSB0 flash monitor
```

---

## Development Team

* Ana Paola Cardona Valenzuela — 22061031
* Emiliano Pérez Dyck — 22061061

**Instructor:** Dr. Juan Alberto Ramírez Quintana  
**Course:** Artificial Intelligence · Team 1  
**Institution:** Instituto Tecnológico de Chihuahua (TecNM / ITCH)  
**Department:** Electrical and Electronic Engineering  
**Date:** April 2026

---

<p align="center">
  Instituto Tecnológico de Chihuahua · Chihuahua, Mexico · 2026
</p>
