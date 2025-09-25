#include "main.h"
#include "esp_log.h"

static const char *TAG = "wifi";

// WiFi параметры
const char* ssid = "your_ssid";
const char* password = "your_password";

static esp_websocket_client_handle_t ws_client;
static char device_id[13] = {0};
static bool handshake_ok = false;

// ---------- Wi-Fi ----------
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "WiFi started, connecting...");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "WiFi disconnected, reconnecting...");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
    }
}

// ---------- mDNS ----------
static void mdns_init_client(void)
{
    ESP_ERROR_CHECK(mdns_init());
    ESP_ERROR_CHECK(mdns_hostname_set("esp32-client"));
    ESP_ERROR_CHECK(mdns_instance_name_set("Driving robot Client"));
    ESP_LOGI(TAG, "mDNS client initialized");
}

static esp_err_t wifi_init(void)
{
    esp_err_t err;
    ESP_LOGI(TAG, "Initializing WiFi...");

    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition error, erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NVS init failed: %s", esp_err_to_name(err));
        return err;
    }

    err = esp_netif_init();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE){
        ESP_LOGE(TAG, "esp_netif_init failed: %s", esp_err_to_name(err));
        return err;
    }

    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE){
        ESP_LOGE(TAG, "esp_event_loop_create_default failed: %s", esp_err_to_name(err));
        return err;
    }

    esp_netif_t *sta_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (sta_netif == NULL){
        sta_netif = esp_netif_create_default_wifi_sta();
        if (sta_netif == NULL){
            ESP_LOGE(TAG, "Failed to create default WiFi STA");
            return ESP_FAIL;
        }
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&cfg);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE){
        ESP_LOGE(TAG, "esp_wifi_init failed: %s", esp_err_to_name(err));
        return err;
    }

    err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE){
        ESP_LOGE(TAG, "Failed to register WiFi event handler");
        return err;
    }

    err = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE){
        ESP_LOGE(TAG, "Failed to register IP event handler");
        return err;
    }

    err = esp_wifi_set_mode(WIFI_MODE_STA);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE){
        ESP_LOGE(TAG, "Failed to set WiFi mode: %s", esp_err_to_name(err));
        return err;
    }

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = ssid,
            .password = password,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg.capable = true,
            .pmf_cfg.required = false,
        },
    };

    err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (err != ESP_OK){
        ESP_LOGE(TAG, "Failed to set WiFi config: %s", esp_err_to_name(err));
        return err;
    }

    err = esp_wifi_start();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE){
        ESP_LOGE(TAG, "Failed to start WiFi: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "WiFi initialized and started");

    mdns_init_client();

    return ESP_OK;
}

// ---------- Получение MAC ----------
static void get_device_id()
{
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    snprintf(device_id, sizeof(device_id), "%02X%02X%02X%02X%02X%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    ESP_LOGI(TAG, "Device ID: %s", device_id);
}

esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    ESP_LOGI(TAG, "HTTP event received, id=%d", evt->event_id);
    return ESP_OK;
}

void download_command_from_server()
{
    ESP_LOGI(TAG, "Downloading command from server...");
    // (оставил как в исходном коде, только добавлены логи)
}

// ---------- WebSocket обработчик ----------
static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, esp_websocket_event_data_t *event_data)
{
    ESP_LOGI(TAG, "WebSocket event id=%d", event_id);
    char expected[20];

    if (event_id == WEBSOCKET_EVENT_DATA) {
        snprintf(expected, sizeof(expected), "OK:%s", device_id);

        if (strncmp((char *)event_data->data_ptr, expected, strlen(expected)) == 0) {
            handshake_ok = true;
            ESP_LOGI(TAG, "Handshake OK with server");
        }
        else if (strncmp((char *)event_data->data_ptr, "load_command", strlen("load_command")) == 0) {
            ESP_LOGI(TAG, "Received load_command from server");
            download_command_from_server();
        }
        else {
            ESP_LOGW(TAG, "Unexpected WebSocket data: %.*s", event_data->data_len, (char *)event_data->data_ptr);
        }
    }
}

// ---------- Поиск и подключение к серверу ----------
static esp_err_t find_and_connect_server()
{
    ESP_LOGI(TAG, "Searching for server via mDNS...");
    mdns_result_t *results = NULL;
    esp_err_t err = mdns_query_ptr("_command", "_tcp", 3000, 10, &results);

    if (err) {
        ESP_LOGE(TAG, "mDNS query failed: %s", esp_err_to_name(err));
        return ESP_FAIL;
    }
    if (!results) {
        ESP_LOGW(TAG, "No mDNS results found");
        return ESP_FAIL;
    }

    for (mdns_result_t *r = results; r; r = r->next) {
        char addr[64];
        inet_ntoa_r(((struct sockaddr_in *)r->addr)->sin_addr, addr, sizeof(addr));
        int port = r->port;
        ESP_LOGI(TAG, "Found server %s:%d", addr, port);

        char ws_uri[128];
        snprintf(ws_uri, sizeof(ws_uri), "ws://%s:%d/ws", addr, port);

        esp_websocket_client_config_t websocket_cfg = {
            .uri = ws_uri,
        };

        ws_client = esp_websocket_client_init(&websocket_cfg);
        if (ws_client == NULL) {
            ESP_LOGE(TAG, "Failed to init WebSocket client");
            mdns_query_results_free(results);
            return ESP_FAIL;
        }

        err = esp_websocket_register_events(ws_client, WEBSOCKET_EVENT_ANY, websocket_event_handler, NULL);
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE){
            ESP_LOGE(TAG, "Failed to register WebSocket events");
            mdns_query_results_free(results);
            return err;
        }

        err = esp_websocket_client_start(ws_client);
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE){
            ESP_LOGE(TAG, "Failed to start WebSocket client");
            mdns_query_results_free(results);
            return err;
        }

        vTaskDelay(pdMS_TO_TICKS(1000));

        if (esp_websocket_client_is_connected(ws_client)) {
            ESP_LOGI(TAG, "WebSocket connected, sending device ID");
            esp_websocket_client_send_text(ws_client, device_id, strlen(device_id), portMAX_DELAY);

            handshake_ok = false;
            vTaskDelay(pdMS_TO_TICKS(1000));

            if (handshake_ok) {
                ESP_LOGI(TAG, "Handshake successful with server");
                mdns_query_results_free(results);
                return true;
            } else {
                ESP_LOGW(TAG, "Handshake failed, closing WebSocket");
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
    bool connected = (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK);
    ESP_LOGI(TAG, "wifi_is_connected: %d", connected);
    return connected;
}

void connect_wifi(void)
{
    ESP_LOGI(TAG, "Connecting to WiFi and server...");
    while (!find_and_connect_server()) {
        ESP_LOGW(TAG, "Retrying server connection in 5s...");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// ======== MAIN APP ========
void wifi_main(void)
{
    ESP_LOGI(TAG, "Starting WiFi main");
    init_filesystem();
    get_device_id();

    while (wifi_init() != ESP_OK) {
        ESP_LOGW(TAG, "WiFi init failed, retrying...");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    vTaskDelay(pdMS_TO_TICKS(5000));
    connect_wifi();

    ESP_LOGI(TAG, "Starting executor_task");
    xTaskCreate(&executor_task, "executor_task", 8192, NULL, 5, NULL);
}
