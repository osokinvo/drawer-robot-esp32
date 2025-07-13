// Обработка команд движения (G0, G1, G2, G3). Управление скоростью (F), выбор плоскости (G17, G18, G19). Работа с координатами (G90/G91 – абсолютные/относительные).

#include "main.h"

// Пропустить пробелы
static const char* skip_spaces(const char* p) {
    while (*p == ' ' || *p == '\t') p++;
    return p;
}

// Считать float после символа (например X10.2)
static bool parse_float_value(const char** p, float* out_val) {
    char* end;
    float val = strtof(*p, &end);

    if (end == *p)
        return false;
    *out_val = val;
    *p = end;
    return true;
}

bool parse_gcode_line(const char* line, gcode_command_t* out_cmd) {
    memset(out_cmd, 0, sizeof(gcode_command_t));
    out_cmd->motion = MOTION_NONE;

    const char* p = line;
    p = skip_spaces(p);

    while (*p) {
        char letter = toupper(*p++);
        p = skip_spaces(p);

        float value;
        if (!parse_float_value(&p, &value)) return false;

        switch (letter) {
            case 'G':
                if ((int)value == 0) {
                    out_cmd->motion = MOTION_G0;
                } else if ((int)value == 1) {
                    out_cmd->motion = MOTION_G1;
                } else {
                    return false;  // не поддерживаемые команды
                }
                break;
            case 'X':
                out_cmd->x = value;
                out_cmd->x_set = true;
                break;
            case 'Y':
                out_cmd->y = value;
                out_cmd->y_set = true;
                break;
            case 'F':
                out_cmd->feed_rate = value;
                out_cmd->feed_rate_set = true;
                break;
            default:
                return false;  // не поддерживаемый символ
        }

        p = skip_spaces(p);
    }

    return true;
}