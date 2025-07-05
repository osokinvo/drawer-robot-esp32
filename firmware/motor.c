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

int position_control_stepper(RobotParams *params, int step_pin, int dir_pin, int prev_speed, intbrake_flag) {
    
}

