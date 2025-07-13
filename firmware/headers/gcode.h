#ifndef GCODE_H
#define GCODE_H

#include <stdbool.h>

typedef enum {
    MOTION_NONE,
    MOTION_G0,   // быстрое перемещение
    MOTION_G1    // линейная интерполяция
} motion_mode_t;

typedef struct {
    motion_mode_t motion;
    float x;
    float y;
    bool x_set;
    bool y_set;
} gcode_command_t;

#endif