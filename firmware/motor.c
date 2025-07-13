// Этот файл содержит функции, которые управляют двигателями, такие как функции для управления скоростью, направлением и т. д.

#include "main.h"

int lock_axis(RobotParams *params, int step_pin, int dir_pin, int *brake_flag) {
// Locks the axis by closing the mechanical brake.
// The function checks the state of the brake flag and changes the signals on
// the pins to unlock the axle.

// @param params Pointer to the structure containing the parameters of the
//               robot.
// @param step_pin The pin connected to the step of the motor.
// @param dir_pin The pin connected to the direction of the motor.
// @param brake_flag Pointer to the brake flag of the motor.
//
// @return OK if the axis was successfully locked, otherwise an error code.
    get_brake_flags(params);
    if (*brake_flag != True){
        gpio_set_level(dir_pin, CLOSE_BRAKE_DIR);
        while (*brake_flag != True){
            gpio_set_level(step_pin, HIGH);
            delayMicroseconds(STEP_MIN_PERIOD);
            gpio_set_level(step_pin, LOW);
            delayMicroseconds(params->step_low_brake);
            get_brake_flags(params);
        }
    }
    return OK;
}

int unlock_axis(RobotParams *params, int step_pin, int dir_pin, int *brake_flag) {
// Разблокирует ось, открывая механический тормоз.
// Функция проверяет состояние флага тормоза и изменяет сигналы на пинах, чтобы
// разблокировать ось.
//
// @param params Указатель на структуру с параметрами робота.
// @param step_pin Пин, на котором подключен шаг двигателя тормоза.
// @param dir_pin Пин, на котором подключен сигнал направления двигателя тормоза.
// @param brake_flag Указатель на флаг открытия тормоза.
//  
// @return OK Возвращает OK, когда ось успешно разблокируется.
    get_brake_flags(params);
    if (*brake_flag != True){
        gpio_set_level(dir_pin, OPEN_BRAKE_DIR);
        while (*brake_flag != True){
            gpio_set_level(step_pin, HIGH);
            delayMicroseconds(STEP_MIN_PERIOD);
            gpio_set_level(step_pin, LOW);
            delayMicroseconds(params->step_low_brake);
            get_brake_flags(params);
        }
    }
    return OK;
}

void motor_control_step(motor_control_t* motor) {
    // Считаем ошибку между планом и энкодером
    float error = motor->planned_position_steps - motor->encoder_position_steps * MOTOR_STEP_PER_RATATION / ENCODER_STEP_PER_RATATION;

    // Коррекция скорости по отставанию/опережению
    if (error > 0.0f) {
        // Двигатель отстаёт — ускоряемся
        if (motor->speed_adjustment < MAX_ADJUSTMENT)
            motor->speed_adjustment += ADJUSTMENT_STEP;
    } else if (error < 0.0f) {
        // Двигатель опережает — замедляемся
        if (motor->speed_adjustment > -MAX_ADJUSTMENT)
            motor->speed_adjustment -= ADJUSTMENT_STEP;
    }

    // Задержка между шагами с учётом коррекции
    float delay_us = motor->base_step_delay_us * (1.0f - motor->speed_adjustment);

    // Отправляем шаг
    gpio_set_level(motor->step_pin, 1);
}

int position_control_stepper(motor_control_t *motor, int prev_speed, int accel) {
    // Задать направление на основе цели
    if (motor->target_position > motor->planned_position)
        motor->direction = 1;
    else
        motor->direction = -1;

    // Шагаем до достижения цели
    while ((motor->direction > 0 && motor->planned_position < motor->target_position) ||
           (motor->direction < 0 && motor->planned_position > motor->target_position)) {
        motor->step_delay_us = 10000000 / prev_speed;
        motor_control_step(motor);
        motor->planned_position += motor->direction;
        prev_speed += accel * motor->step_delay_us / 1000000;
    }

    return OK;
}

