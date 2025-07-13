// Этот файл содержит функции, которые работают с энкодерами, такие как функции для чтения данных с энкодеров и т. д.

#include "esp_log.h"

static const char *TAG = "Encoder";

// Конфигурация энкодера
#define PCNT_HIGH_LIMIT 1000
#define PCNT_LOW_LIMIT   -1000

// Пины энкодера (замените на свои)
#define ENCODER_A_GPIO   GPIO_NUM_4
#define ENCODER_B_GPIO   GPIO_NUM_5

// Очередь для событий от PCNT
static QueueHandle_t pcnt_evt_queue = NULL;

// Структура для событий энкодера
typedef struct {
    int unit;  // PCNT unit
    uint32_t status; // Статус
} pcnt_evt_t;

// Обработчик прерываний PCNT
static void IRAM_ATTR pcnt_intr_handler(void *arg) {
    uint32_t intr_status = PCNT.int_st.val;
    for (int i = 0; i < PCNT_UNIT_MAX; i++) {
        if (intr_status & (BIT(i))) {
            pcnt_evt_t evt;
            evt.unit = i;
            evt.status = PCNT.status_unit[i].val;
            xQueueSendFromISR(pcnt_evt_queue, &evt, NULL);
            PCNT.int_clr.val = BIT(i);
        }
    }
}

// Инициализация PCNT для энкодера
static void init_pcnt(int unit, int gpio_a, int gpio_b) {
    pcnt_config_t pcnt_config = {
        .pulse_gpio_num = gpio_a,
        .ctrl_gpio_num = gpio_b,
        .lctrl_mode = PCNT_MODE_REVERSE,
        .hctrl_mode = PCNT_MODE_KEEP,
        .pos_mode = PCNT_COUNT_INC,
        .neg_mode = PCNT_COUNT_DEC,
        .counter_h_lim = PCNT_HIGH_LIMIT,
        .counter_l_lim = PCNT_LOW_LIMIT,
        .unit = unit,
        .channel = PCNT_CHANNEL_0,
    };
    pcnt_unit_config(&pcnt_config);

    pcnt_config.pulse_gpio_num = gpio_b;
    pcnt_config.ctrl_gpio_num = gpio_a;
    pcnt_config.channel = PCNT_CHANNEL_1;
    pcnt_config.pos_mode = PCNT_COUNT_DEC;
    pcnt_config.neg_mode = PCNT_COUNT_INC;
    pcnt_unit_config(&pcnt_config);

    pcnt_counter_pause(unit);
    pcnt_counter_clear(unit);
    pcnt_event_enable(unit, PCNT_EVT_H_LIM);
    pcnt_event_enable(unit, PCNT_EVT_L_LIM);
    pcnt_isr_register(pcnt_intr_handler, NULL, 0, NULL);
    pcnt_intr_enable(unit);
    pcnt_counter_resume(unit);
}

// Задача для обработки событий энкодера
static void encoder_task(void *arg) {
    pcnt_evt_t evt;
    int16_t count = 0;
    while (1) {
        if (xQueueReceive(pcnt_evt_queue, &evt, portMAX_DELAY)) {
            pcnt_get_counter_value(evt.unit, &count);
            ESP_LOGI(TAG, "Encoder count: %d", count);
            // Здесь можно добавить обработку значений (например, управление двигателем)
        }
    }
}

// Инициализация энкодера
void encoder_init() {
    // Создаем очередь для событий
    pcnt_evt_queue = xQueueCreate(10, sizeof(pcnt_evt_t));
    if (!pcnt_evt_queue) {
        ESP_LOGE(TAG, "Failed to create queue");
        return;
    }

    // Инициализируем PCNT для энкодера
    init_pcnt(PCNT_UNIT_0, ENCODER_A_GPIO, ENCODER_B_GPIO);

    // Создаем задачу для обработки событий
    xTaskCreate(encoder_task, "encoder_task", 2048, NULL, 5, NULL);
}