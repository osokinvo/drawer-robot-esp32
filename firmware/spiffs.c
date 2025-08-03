#include "main.h"

static size_t get_spiffs_free_space()
{
    size_t total = 0;
    size_tused = 0;

    esp_spiffs_info(NULL, &total, &used);
    return total - used;
}

robot_err_t save_cmd_to_spiffs(cmd_block_t *first_block, uint16_t cmd_count)
{
// Созраняет массив команд в постоянную память. Первое число файла - количество команд,
// остальные - массив структур.
// В качестве параметров принимает:
// first_block - указатель на первый элемент массива команд
// cmd_count - количество элементов в массиве
//
// Возвращает:
// ROBOT_OK - успешно сохранено
// ROBOT_ERR_NO_MEM - нет свободной памяти
// ROBOT_FAIL - не удалось открыть / создать файл

    size_t length;
    FILE *f = fopen(CMD_FILE, "wb");
    
    length = sizeof(uint16_t) + cmd_count * sizeof(cmd_block_t);
    if (f == NULL) {
        return ROBOT_FAIL; // не удалось открыть файл
    }

    if (length > get_spiffs_free_space()) {
        return ROBOT_ERR_NO_MEM;
    }
    // Записываем количество команд (для восстановления)
    fwrite(&cmd_count, sizeof(uint16_t), 1, f);

    // Записываем массив структур
    fwrite(first_block, sizeof(gcode_cmd_t), cmd_count, f);
    fclose(f);

    return ROBOT_OK;
}

robot_err_t load_gcode_from_spiffs(cnd_cache_t *cache)
{
// Созраняет массив команд в быструю память. Первое число файла - количество команд,
// остальные - массив структур.
// В качестве параметров принимает:
// cache - указатель на структуру команд
//
// Возвращает:
// ROBOT_OK - успешно сохранено
// ROBOT_ERR_NO_MEM - нет свободной памяти
// ROBOT_FAIL - не удалось открыть / создать файл
    size_t count;
    cmd_block_t *commands;
    FILE *f = fopen(CMD_FILE, "rb");

    if (f == NULL) {
        return ROBOT_FAIL; // файла нет
    }

    fread(&count, sizeof(size_t), 1, f);

    // Выделяем память под массив
    commands = malloc(count * sizeof(gcode_cmd_t));
    if (!commands) {
        fclose(f);
        return ROBOT_ERR_NO_MEM;
    }

    read = fread(commands, sizeof(gcode_cmd_t), count, f);
    fclose(f);

    cache->cmds = commands;
    cache->cmd_count = count;

    return ESP_OK;
}

robot_err_t *cmd_file_exists(void)
{
    if (access(CMD_FILE, F_OK) == 0) {
        return ESP_OK;
    return ESP_FAIL;
}


// ======== INIT FILE SYSTEM ========
void init_filesystem()
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",         // Точка монтирования
        .partition_label = NULL,        // Используется стандартный раздел
        .max_files = 5,                 // Максимальное количество открытых файлов
        .format_if_mount_failed = true  // Форматировать если не удалось монтировать
    };
    esp_vfs_spiffs_register(&conf);
}
