// Этот файл содержит функции, которые работают с энкодерами, такие как функции для чтения данных с энкодеров и т. д.

#include "main.h"

static void IRAM_ATTR encoder_isr_handler(void* arg) {
    encoder_t* encoder = (encoder_t*)arg;
    
    // Читаем текущее состояние
    uint8_t new_state = (gpio_get_level(encoder->pin_a) << 1) | 
                        gpio_get_level(encoder->pin_b);
    
    // Определяем направление
    encoder->motor->encoder_position_steps += gray_transitions[encoder->prev_state][new_state];;
    
    encoder->prev_state = new_state;
}

// Инициализация энкодера
void encoder_init(encoder_t* enc, gpio_num_t pin_a, gpio_num_t pin_b) {
    enc->pin_a = pin_a;
    enc->pin_b = pin_b;
    enc->count = 0;
    enc->prev_state = 0;

    // Настройка GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << pin_a),
        .mode = GPIO_MODE_INPUT,
        .intr_type = GGPIO_INTR_POSEDGE,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };
    gpio_config(&io_conf);

    io_conf.pin_bit_mask = (1ULL << pin_b);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    // Регистрация обработчика с передачей указателя на структуру
    gpio_isr_handler_add(pin_a, encoder_isr_handler, enc);
}
