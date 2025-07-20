#ifndef MOTOR_H
#define MOTOR_H

#define ADJUSTMENT_STEP 0.02                // шаг коррекции скорости
#define MAX_ADJUSTMENT 0.1                  // максимальная коррекция скорости

typedef struct {
    int step_pin;
    int dir_pin;

    float base_acceleration_steps_s;        // базовое ускорение (шагов в секунду)
    float step_delay_us;                    // задержка между шагами (скорость, мкс)

    float speed_adjustment;                 // текущая коррекция скорости [-0.1 .. 0.1]

    int direction;                          // 1 = вперёд, -1 = назад

    int target_position_steps;              // абсолютная целевая координата (мм)
    int planned_position_steps;             // где сейчас план по G-коду (шаги)
    volatile int encoder_position_steps;    // где физически находится (шаги)
    
} motor_control_t;


#endif

