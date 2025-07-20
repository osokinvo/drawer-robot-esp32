#ifndef ENCODER_H
#define ENCODER_H

// Структура энкодера
typedef struct {
    gpio_num_t pin_a;
    gpio_num_t pin_b;
    motor_control_t *motor;
    uint8_t prev_state;
} encoder_t;

// Таблица переходов между состояниями (A, B)
// prev_state: [0b00, 0b01, 0b10, 0b11] → new_state: [0b00, 0b01, 0b10, 0b11]
const int8_t gray_transition_table[] = {
        0,  +1, -1,  0,  // Из 00 (0)
        -1,  0,  0, +1,  // Из 01 (1)
        +1,  0,  0, -1,  // Из 10 (2)
        0, -1, +1,  0    // Из 11 (3)
};

void encoder_init(encoder_t* enc, gpio_num_t pin_a, gpio_num_t pin_b);

#endif // ENCODER_HANDLER_H