//Этот файл содержит функции, которые работают с WiFi соединением, такие как подключение к сети, отправка и получение данных.
#include "main.h"


// WiFi параметры
const char* ssid = "your_ssid";
const char* password = "your_password";
static EventGroupHandle_t s_wifi_event_group;
static const char *TAG = "GCODE_SYSTEM";

static esp_websocket_client_handle_t ws_client;
static char device_id[13] = {0};
static bool handshake_ok = false;


// ---------- Wi-Fi ----------
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Disconnected. Reconnecting...");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
    }
}

static void wifi_init(void)
{
    
     // Инициализация TCP/IP стекa и сетевого интерфейса
    ESP_ERROR_CHECK(esp_netif_init());

    // Создаём и запускаем дефолтный цикл обработки событий (события Wi-Fi, IP и т.п.)
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Создаём сетевой интерфейс Wi-Fi в режиме станции (STA)
    esp_netif_create_default_wifi_sta();

    // Конфигурация Wi-Fi с настройками по умолчанию
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    // Инициализация драйвера Wi-Fi с указанной конфигурацией
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Регистрируем обработчик событий Wi-Fi (подключение, отключение и т.п.)
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

    // Регистрируем обработчик событий IP (получение IP адреса и т.п.)
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    // Конфигурация параметров подключения Wi-Fi: SSID, пароль и уровень защиты
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = ssid,              // Имя Wi-Fi сети
            .password = password,          // Пароль от сети
            .threshold.authmode = WIFI_AUTH_WPA2_PSK, // Минимальный уровень защиты
        },
    };

    // Устанавливаем режим работы Wi-Fi — станция (подключается к роутеру)
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    // Передаём конфигурацию в драйвер Wi-Fi
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    // Запускаем Wi-Fi
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi initialization finished.");;
}

// ---------- Получение MAC ----------
static void get_device_id()
{
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    snprintf(device_id, sizeof(device_id), "%02X%02X%02X%02X%02X%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    ESP_LOGI(TAG, "Device ID (MAC): %s", device_id);
}

// ---------- WebSocket обработчик ----------
static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, esp_websocket_event_data_t *event_data)
{
    char expected[20];

    if (event_id == WEBSOCKET_EVENT_CONNECTED) {
        ESP_LOGI(TAG, "WebSocket connected");
    }
    else if (event_id == WEBSOCKET_EVENT_DATA) {
        ESP_LOGI(TAG, "Received: %.*s", event_data->data_len, (char *)event_data->data_ptr);

        // Проверяем ответ: "OK:<device_id>"
        snprintf(expected, sizeof(expected), "OK:%s", device_id);

        if (strncmp((char *)event_data->data_ptr, expected, strlen(expected)) == 0) {
            ESP_LOGI(TAG, "Handshake successful");
            handshake_ok = true;
        }
    }
}

// ---------- mDNS ----------
static void mdns_init_client(void)
{
    ESP_ERROR_CHECK(mdns_init());
    ESP_ERROR_CHECK(mdns_hostname_set("esp32-client"));
    ESP_ERROR_CHECK(mdns_instance_name_set("ESP32 GCODE Client"));
    ESP_LOGI(TAG, "mDNS initialized");
}

// ---------- Поиск и подключение к серверу ----------
static bool find_and_connect_server()
{
    ESP_LOGI(TAG, "Searching for servers with service _gcode._tcp.local ...");

    mdns_result_t *results = NULL;
    esp_err_t err = mdns_query_ptr("_gcode", "_tcp", 3000, 10, &results);

    if (err) {
        ESP_LOGE(TAG, "mDNS query failed: %s", esp_err_to_name(err));
        return false;
    }
    if (!results) {
        ESP_LOGW(TAG, "No servers found");
        return false;
    }

    for (mdns_result_t *r = results; r; r = r->next) {
        char addr[64];
        inet_ntoa_r(((struct sockaddr_in *)r->addr)->sin_addr, addr, sizeof(addr));
        int port = r->port;

        ESP_LOGI(TAG, "Trying server: %s:%d", addr, port);

        // Формируем URI WebSocket
        char ws_uri[128];
        snprintf(ws_uri, sizeof(ws_uri), "ws://%s:%d/ws", addr, port);

        esp_websocket_client_config_t websocket_cfg = {
            .uri = ws_uri,
        };

        ws_client = esp_websocket_client_init(&websocket_cfg);
        esp_websocket_register_events(ws_client, WEBSOCKET_EVENT_ANY, websocket_event_handler, NULL);
        esp_websocket_client_start(ws_client);

        // Ждём соединения
        vTaskDelay(pdMS_TO_TICKS(1000));

        if (esp_websocket_client_is_connected(ws_client)) {
            // Отправляем ID
            esp_websocket_client_send_text(ws_client, device_id, strlen(device_id), portMAX_DELAY);

            // Ждём ответ
            handshake_ok = false;
            vTaskDelay(pdMS_TO_TICKS(1000));

            if (handshake_ok) {
                ESP_LOGI(TAG, "Server confirmed handshake");
                mdns_query_results_free(results);
                return true;
            } else {
                ESP_LOGW(TAG, "Handshake failed, closing connection");
                esp_websocket_client_stop(ws_client);
                esp_websocket_client_destroy(ws_client);
            }
        }
    }

    mdns_query_results_free(results);
    return false;
}

bool wifi_is_connected()
{
    wifi_ap_record_t ap_info;
    return (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK);
}

// ======== INIT FILE SYSTEM ========
void init_filesystem()
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };
    esp_vfs_spiffs_register(&conf);
    ESP_LOGI(TAG, "SPIFFS mounted");
}

// ======== READ LINE FROM FLASH ========
bool read_comand_code_line_from_flash(FILE *f, char *buffer, size_t bufsize)
{
    if (fgets(buffer, bufsize, f) != NULL) {
        return true;
    }
    return false;
}

// ======== READ LINE FROM TF-CARD ========
bool read_gcode_line_from_sd(FILE *f, char *buffer, size_t bufsize)
{
    if (fgets(buffer, bufsize, f) != NULL) {
        return true;
    }
    return false;
}

// ======== EXECUTOR TASK ========
void executor_task(void *pvParameters)
{
    char line[128];
    FILE *gcode_file = fopen("/spiffs/gcode.txt", "r");
    if (!gcode_file) {
        ESP_LOGE(TAG, "G-code file not found!");
        vTaskDelete(NULL);
        return;
    }

    while (1) {
        if (wifi_is_connected()) {
            ESP_LOGI(TAG, "[Wi-Fi] Reading commands from server...");
            // TODO: Реализовать получение G-кода по Wi-Fi (HTTP/WebSocket)
            // Пока имитация:
            vTaskDelay(pdMS_TO_TICKS(1000));
        } else {
            ESP_LOGW(TAG, "[Offline] Executing G-code from flash...");
            if (read_gcode_line_from_flash(gcode_file, line, sizeof(line))) {
                ESP_LOGI(TAG, "Executing: %s", line);
                // TODO: Выполнить команду G-кода
                vTaskDelay(pdMS_TO_TICKS(500)); // Имитация выполнения
            } else {
                ESP_LOGI(TAG, "End of G-code file");
                break;
            }
        }
    }

    fclose(gcode_file);
    vTaskDelete(NULL);
}

// ======== MAIN APP ========
void wifi_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_sta();
    init_filesystem();

    wifi_init();

    vTaskDelay(pdMS_TO_TICKS(5000)); // ждём IP

    get_device_id();
    mdns_init_client();

    while (!find_and_connect_server()) {
        ESP_LOGW(TAG, "No valid server found, retrying...");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }

    ESP_LOGI(TAG, "Connected to server successfully. Ready for commands...");

    // Запуск основной задачи
    xTaskCreate(&executor_task, "executor_task", 8192, NULL, 5, NULL);
}
