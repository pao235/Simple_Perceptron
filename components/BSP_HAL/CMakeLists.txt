idf_component_register(SRCS "ADC.c" "GPIO.c" "PRINT.c"
                    INCLUDE_DIRS "include"
                     REQUIRES driver esp_adc esp_timer driver esp_adc
                     )

