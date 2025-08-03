#ifndef SPIFFS_H
#define SPIFFS_H

#include <stdio.h>
#include "esp_spiffs.h"

#define CMD_FILE "/spiffs/cmd.bin"

static size_t get_spiffs_free_space();

robot_err_t save_cmd_to_spiffs(cmd_block_t *first_block, uint16_t cmd_count);

robot_err_t load_gcode_from_spiffs(cnd_cache_t *cache);

robot_err_t *cmd_file_exists(void);

void init_filesystem();

#endif