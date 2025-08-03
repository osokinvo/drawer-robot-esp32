// Этот файл содержит функции для чтения и установки входов и выходов для измерения усилий с датчиков.

#include "main.h"

void force_input_init(int pin)
{
    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(pin, ADC_ATTEN_11db);
}


int get_force(int pin)
{
    return adc1_get_raw(pin);
}

void force_output_init(int pin)
{
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = PWM_RES,
        .timer_num = PWM_TIMER,
        .freq_hz = PWM_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);
    
    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = PWM_CHANNEL,
        .timer_sel = PWM_TIMER,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = pin,
        .duty = 0,
        .hpoint = 0,
        .flags.output_invert = 0
    };
    ledc_channel_config(&ledc_channel);
}

void set_force(int force, int channel)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, channel, force);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, channel);
}
