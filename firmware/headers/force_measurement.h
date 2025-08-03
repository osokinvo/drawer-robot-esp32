#ifndef FORCE_MEASUREMENT_H
#define FORCE_MEASUREMENT_H

#define PWM_CHANNEL     LEDC_CHANNEL_0
#define PWM_TIMER       LEDC_TIMER_0
#define PWM_FREQ_HZ     10000
#define PWM_RES         LEDC_TIMER_13_BIT

void force_input_init(int pin);

int get_force(int pin);

void force_output_init(int pin);

void set_force(int force, int channel);

#endif