#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include <math.h>
#include "encoder_handler.h"
#include "driver/pcnt.h"
#include "freertos/queue.h"

#include "main.h"
#include "gcode.h"

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

// =============================================
// Управление двигателями (ШИМ/цифровые выходы)
// =============================================
#define Z_AXIS_MOTOR_PWM_PIN        23  // GPIO23 - ШИМ сигнал скорости двигателя оси Z
#define Z_AXIS_MOTOR_CTRL_PIN       22  // GPIO22 - Сигнал включения двигателя оси Z
#define Z_AXIS_MOTOR_DIR_PIN         1  // GPIO1  - Направление вращения двигателя оси Z

// =============================================
// Управление тормозами (цифровые выходы)
// =============================================
#define V_AXIS_BRAKE_DIR_PIN         3  // GPIO3  - Направление тормоза оси V
#define V_AXIS_BRAKE_STEP_PIN       21  // GPIO21 - Шаг управления тормозом оси V
#define V_AXIS_MOTOR_DIR_PIN        19  // GPIO19 - Направление двигателя оси V
#define V_AXIS_MOTOR_STEP_PIN       18  // GPIO18 - Шаг двигателя оси V

// =============================================
// I2C (датчики/дисплей)
// =============================================
#define I2C_SCL_PIN                 5  // GPIO5  - Тактовая линия I2C
#define I2C_SDA_PIN                17  // GPIO17 - Линия данных I2C

// =============================================
// Управление осью U (цифровые выходы)
// =============================================
#define U_AXIS_MOTOR_STEP_PIN      16  // GPIO16 - Шаг двигателя оси U
#define U_AXIS_MOTOR_DIR_PIN        4  // GPIO4  - Направление двигателя оси U
#define U_AXIS_BRAKE_STEP_PIN       2  // GPIO2  - Шаг тормоза оси U
#define U_AXIS_BRAKE_DIR_PIN       15  // GPIO15 - Направление тормоза оси U

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

typedef struct {
    int max_speed_U;        // максимальная скорость мотора U
    int max_speed_V;        // максимальная скорость мотора V
    int max_accel_U;        // максимальное линейное ускорение U
    int max_accel_V;        // максимальное линейное ускорение V
    int accel_U;            // линейное ускорение U
    int accel_V;            // линейное ускорение V

    int speed_U_brake;       // скорость остановки U
    int speed_V_brake;       // скорость остановки V

    int max_U_brake;         // максимальное положение тормоза U
    int min_U_brake;         // минимальное положение тормоза U
    int max_V_brake;         // максимальное положение тормоза V
    int min_V_brake;         // минимальное положение тормоза V

    int step_delay_brake;  // период шага двигателя тормоза, мкс
    int step_low_brake;    // период низкого положения STEP двигателя тормоза, мкс


} RobotParams;


// =============================================
// motor.c
// =============================================

int lock_axis(RobotParams *params, int step_pin, int dir_pin, int *brake_flag);
// Блокирует ось, закрывая механический тормоз. Возвращает ОК, когда ось заблокируется.

void unlock_axis(RobotParams *params, int step_pin, int dir_pin, int *brake_flag);
// Разблокирует ось, открывая механический тормоз. Возвращает ОК, когда ось разблокируется.

#endif