// Этот файл содержит функции для чтения и установки входов и выходов для измерения усилий с датчиков.

#include "main.h"

void force_input_init(int pin){
    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(pin, ADC_ATTEN_11db);
}

void force_output_init(int pin){
}

int get_force(int pin){
    return adc1_get_raw(pin);
}