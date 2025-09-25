#ifndef MAIN_H
#define MAIN_H

#include <ctype.h>
#include <queue.h>
#include <math.h>
#include <semphr.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <task.h>
#include <WiFi.h>

#include "driver/gpio.h"
#include "driver/pcnt.h"
// #include "driver/sdmmc_host.h" // TF-card (закомментировано)
#include "esp_timer.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_spiffs.h"
#include "esp_system.h"
#include "esp_websocket_client.h"
#include "nvs_flash.h"
#include "mdns.h"
#include <FreeRTOS.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
// #include "sdmmc_cmd.h" // TF-card (закомментировано)

#include "encoder.h"
#include "force_measurement.h"
#include "gcode.h"
#include "motor.h"
#include "spiffs.h"
#include "wifi.h"

#define ENABLE_DEBUG_LOGS 1

#if ENABLE_DEBUG_LOGS

#include "esp_log.h"
#define LOGI(tag, ftm, ...) ESP_LOGI(tag, ftm, ##__VA_ARGS__)
#define LOGW(tag, ftm, ...) ESP_LOGW(tag, ftm, ##__VA_ARGS__)
#define LOGE(tag, ftm, ...) ESP_LOGE(tag, ftm, ##__VA_ARGS__)

#else

#define LOGI(tag, ftm, ...) {}
#define LOGW(tag, ftm, ...) {}
#define LOGE(tag, ftm, ...) {}

#endif


// =============================================
// Аналоговые входы (ADC1, 12-бит)
// =============================================
#define U_AXIS_FORCE_ADC_CHANNEL    ADC1_CHANNEL_0  // GPIO36 - Вход усилия по оси U (операционный усилитель)
#define V_AXIS_FORCE_ADC_CHANNEL    ADC1_CHANNEL_3  // GPIO39 - Вход усилия по оси V (операционный усилитель)
#define Z_AXIS_FORCE_ADC_CHANNEL    ADC1_CHANNEL_6  // GPIO34 - Вход усилия по оси Z (операционный усилитель)

// =============================================
// Аналоговый выход (DAC, 8-бит)
// =============================================
#define Z_AXIS_FORCE_DAC_CHANNEL    DAC_CHANNEL_1    // GPIO32 - ЦАП для задания усилия по оси Z

// =============================================
// Цифровые входы 
// =============================================
#define Z_AXIS_LIMIT_MAX_PIN        35  // GPIO35 - Концевой датчик max положения оси Z
#define Z_AXIS_LIMIT_MIN_PIN        33  // GPIO33 - Концевой датчик min положения оси Z

// =============================================
// Энкодеры (цифровые входы)
// =============================================
#define U_AXIS_ENC_A_PIN            25  // GPIO25 - Фаза A энкодера оси U
#define U_AXIS_ENC_B_PIN            26  // GPIO26 - Фаза B энкодера оси U
#define U_AXIS_ENC_X_PIN            27  // GPIO27 - Индексный сигнал энкодера оси U

#define V_AXIS_ENC_A_PIN            12  // GPIO12 - Фаза A энкодера оси V
#define V_AXIS_ENC_B_PIN            14  // GPIO14 - Фаза B энкодера оси V
#define V_AXIS_ENC_X_PIN            13  // GPIO13 - Индексный сигнал энкодера оси V

#if ENABLE_DEBUG_LOGS == 0

// =============================================
// Управление двигателями (ШИМ/цифровые выходы)
// =============================================
#define Z_AXIS_MOTOR_PWM_PIN        23  // GPIO23 - ШИМ сигнал скорости двигателя оси Z
#define Z_AXIS_MOTOR_CTRL_PIN       22  // GPIO22 - Сигнал включения двигателя оси Z
#define Z_AXIS_MOTOR_DIR_PIN         1  // GPIO1  - Направление вращения двигателя оси Z

#endif

// =============================================
// Управление осью U (цифровые выходы)
// =============================================
#define U_AXIS_MOTOR_STEP_PIN      16  // GPIO16 - Шаг двигателя оси U
#define U_AXIS_MOTOR_DIR_PIN        4  // GPIO4  - Направление двигателя оси U
#define U_AXIS_BRAKE_STEP_PIN       2  // GPIO2  - Шаг тормоза оси U
#define U_AXIS_BRAKE_DIR_PIN       15  // GPIO15 - Направление тормоза оси U

#if ENABLE_DEBUG_LOGS == 0

// =============================================
// Управление осью V (цифровые выходы)
// =============================================
#define V_AXIS_BRAKE_DIR_PIN         3  // GPIO3  - Направление тормоза оси V
#define V_AXIS_BRAKE_STEP_PIN       21  // GPIO21 - Шаг управления тормозом оси V
#define V_AXIS_MOTOR_DIR_PIN        19  // GPIO19 - Направление двигателя оси V
#define V_AXIS_MOTOR_STEP_PIN       18  // GPIO18 - Шаг двигателя оси V

#endif

// =============================================
// I2C (датчики/дисплей)
// =============================================
#define I2C_SCL_PIN                 5  // GPIO5  - Тактовая линия I2C
#define I2C_SDA_PIN                17  // GPIO17 - Линия данных I2C

// =============================================
// Конфигурация PCA9555D
// =============================================
#define PCA9555_ADDR         0x20      // Базовый адрес (A0=A1=A2=GND)
#define PCA9555_INPUT_REG_0  0x00      // Регистр ввода PORT0
#define PCA9555_INPUT_REG_1  0x01      // Регистр ввода PORT1
#define PCA9555_CONFIG_REG_0 0x06      // Регистр конфигурации PORT0 (1=input)
#define PCA9555_CONFIG_REG_1 0x07      // Регистр конфигурации PORT1 (1=input)

// =============================================
// Концевые датчики (битовые маски)
// =============================================
#define U_AXIS_MIN_LIMIT     (1 << 0)  // PORT0.0 (IO0_0) Концевой датчик min положения оси U
#define U_AXIS_MAX_LIMIT     (1 << 2)  // PORT0.2 (IO0_2) Концевой датчик max положения оси U
#define V_AXIS_MAX_LIMIT     (1 << 7)  // PORT0.7 (IO0_7) Концевой датчик max положения оси V
#define V_AXIS_MIN_LIMIT     (1 << 1)  // PORT1.1 (IO1_1) Концевой датчик min положения оси V

// =============================================
// Данные двигателей и энкодеров
// =============================================
#define MOTOR_BASE_STEP_PER_RATATION    200     // Базоваое количество шагов на оборот шагового двигателя
#define MICROSTEPS_PER_STEP             1       // Количество микрошагов в шаге
#define MOTOR_STEP_PER_RATATION         MOTOR_BASE_STEP_PER_RATATION * MICROSTEPS_PER_STEP
// Шагов на оборот шагового двигателя
#define MOTOR_MIN_SPEED                 10      // Минимальная скорость двигателя (шагов в секунду)
#define MOTOR_MAX_SPEED                 1000    // Максимальная скорость двигателя (шагов в секунду)
//#define MOTOR_ACCELERATION              100     // Ускорение двигателя

#define BRAKE_MOTOR_STEP_PER_RATATION   200     // Шагов на оборот шагового тормоза
#define ENCODER_STEP_PER_RATATION       2048    // Шагов на оборот энкодера
#define ENCODER2MOTOR_STEP_KOEF MOTOR_STEP_PER_RATATION / ENCODER_STEP_PER_RATATION
// Коэффициент преобразования шагов энкодера в шаги двигателя

#define TRUE 1                         // Признак активации флагов
#define FALSE 0                        // Признак деактивации флагов


#define OK 0                           // Признак успешного выполнения операции
#define ERROR 1                        // Признак неудачного выполнения операции

#define CLOSE_BRAKE_DIR 0                    // Направление активации тормоза
#define OPEN_BRAKE_DIR  1                    // Направление деактивации тормоза

#define STEP_MIN_PERIOD 100             // Минимальная длительность высокого положения STEP шагового двигателя, мкс

typedef enum {
    ROBOT_OK = ESP_OK;                                      // Успешное выполнение
    ROBOT_FAIL = ESP_FAIL;                                  // Общая ошибка
    ROBOT_TIMEOUT = ESP_ERR_TIMEOUT;                        // Таймаут
    ROBOT_ERR_NO_MEM = ESP_ERR_NO_MEM;                      // Ошибка выделения памяти
    ROBOT_ERR_INVALID_ARG = ESP_ERR_INVALID_ARG;            // Неверные аргументы
    ROBOT_ERR_INVALID_STATE = ESP_ERR_INVALID_STATE;        // Неверное состояние
    ROBOT_ERR_INVALID_SIZE = ESP_ERR_INVALID_SIZE;          // Неверный размер
    ROBOT_ERR_NOT_FOUND = ESP_ERR_NOT_FOUND;                // Элемент не найден
    ROBOT_ERR_NOT_SUPPORTED = ESP_ERR_NOT_SUPPORTED;        // Операция или свойство не поддерживаются
    ROBOT_ERR_INVALID_RESPONSE = ESP_ERR_INVALID_RESPONSE;  // Неверный ответ
    ROBOT_ERR_INVALID_CRC = ESP_ERR_INVALID_CRC;            // Неверный CRC или контрольная сумма
    ROBOT_ERR_INVALID_VERSION = ESP_ERR_INVALID_VERSION;    // Неверная версия
    ROBOT_ERR_INVALID_MAC = ESP_ERR_INVALID_MAC;            // Неверный MAC-адрес
    ROBOT_ERR_NOT_FINISHED = ESP_ERR_NOT_FINISHED;          // Операция не завершена
    ROBOT_ERR_NOT_ALLOWED = ESP_ERR_NOT_ALLOWED;            // Операция не разрешена
    ROBOT_ERR_WIFI_BASE = ESP_ERR_WIFI_BASE;                // Стартовый номер ошибок WiFi
    ROBOT_ERR_MESH_BASE = ESP_ERR_MESH_BASE;                // Стартовый номер ошибок MESH
    ROBOT_ERR_FLASH_BASE = ESP_ERR_FLASH_BASE;              // Стартовый номер ошибок FLASH
    ROBOT_ERR_HW_CRYPTO_BASE = ESP_ERR_HW_CRYPTO_BASE;      // Стартовый номер ошибок HW_CRYPTO
    ROBOT_ERR_MEMPROT_BASE = ESP_ERR_MEMPROT_BASE;          // Стартовый номер Memory Protection API
}    robot_err_t;


typedef struct {
    uint32_t u_pos;
    uint32_t v_pos;
    uint16_t prev_u_speed;
    uint16_t prev_v_speed;
    uint16_t u_accel;
    uint16_t v_accel;
}   cmd_block_t;

typedef struct {
    uint16_t cmd_count;
    cmd_block_t *cmds;
}   cnd_cache_t;

typedef struct {
    int speed_U_brake;       // скорость остановки U
    int speed_V_brake;       // скорость остановки V

    int max_U_brake;         // максимальное положение тормоза U
    int min_U_brake;         // минимальное положение тормоза U
    int max_V_brake;         // максимальное положение тормоза V
    int min_V_brake;         // минимальное положение тормоза V

    int step_delay_brake;  // период шага двигателя тормоза, мкс
    int step_low_brake;    // период низкого положения STEP двигателя тормоза, мкс

    cmd_cache_t cache;
    uint16_t current_cmd;

} t_RobotParams;


// =============================================
// stepper.c
// =============================================

int lock_axis(RobotParams *params, int step_pin, int dir_pin, int *brake_flag);
// Блокирует ось, закрывая механический тормоз. Возвращает ОК, когда ось заблокируется.

void unlock_axis(RobotParams *params, int step_pin, int dir_pin, int *brake_flag);
// Разблокирует ось, открывая механический тормоз. Возвращает ОК, когда ось разблокируется.

int position_control_stepper(motor_control_t *motor, int prev_speed, int accel);
// Перемещает шаговый двигатель на участке траектории, заданном в структуре motor_control_t.


#endif