#include "freertos/FreeRTOS.h"     // FreeRTOS base definitions
#include "freertos/task.h"         // For tasks & delays
#include "esp_log.h"               // Logging output
#include "esp_now.h"               // ESP-NOW communication API
#include "esp_wifi.h"              // WiFi configuration
#include "nvs_flash.h"             // Flash storage used by WiFi
#include "driver/adc.h"            // ADC driver for analog-to-digital readings
#include "cJSON.h"                 // JSON library
#include <time.h>                  // Time structures
#include "esp_sntp.h"              // SNTP time syncing (NTP)

// -------------------------
// ADC CONFIG
// -------------------------
// LE250 thermistor analog pin (ADC1 channel 6 = GPIO34)
#define LE250_PIN ADC1_CHANNEL_6

// ID for the device sending the data
#define SENDER_ID 1

// ESP-NOW broadcast MAC (FF:FF:FF:FF:FF:FF sends to all receivers)
static const uint8_t broadcast_mac[ESP_NOW_ETH_ALEN] =
    {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

static const char *TAG = "ESP-NOW";

// -------------------------
// SENSOR READING
// -------------------------
// Reads an analog value from the ADC pin (0–4095 for 12-bit)
int read_sensor() {
    return adc1_get_raw(LE250_PIN);
}

// -------------------------
// JSON CREATION
// -------------------------
// Creates JSON string: {"sensor_id":"1","timestamp":"...","measurement":123}
char* create_sensor_json(const char* sensor_id, int measurement) {
    cJSON *root = cJSON_CreateObject();

    // -------- GET CURRENT TIME --------
    time_t now;
    struct tm timeinfo;

    time(&now);                        // get system time
    localtime_r(&now, &timeinfo);      // convert to struct tm

    // format: 2025-11-18 14:32:01
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &timeinfo);

    // -------- BUILD JSON --------
    cJSON_AddStringToObject(root, "sensor_id", sensor_id);
    cJSON_AddStringToObject(root, "timestamp", time_str);
    cJSON_AddNumberToObject(root, "measurement", measurement);

    // Convert cJSON → char* string
    char *json_str = cJSON_PrintUnformatted(root);

    // Clean up memory
    cJSON_Delete(root);
    return json_str;   // allocated string — caller must free()
}

// -------------------------
// ESP-NOW SEND CALLBACK
// -------------------------
void send_cb(const wifi_tx_info_t *info, esp_now_send_status_t status) {
    ESP_LOGI(TAG, "Send status: %s",
             status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

// -------------------------
// WIFI INIT (REQUIRED FOR ESP-NOW)
// -------------------------
void init_wifi() {
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);                // initialize WiFi stack
    esp_wifi_set_mode(WIFI_MODE_STA);   // station mode
    esp_wifi_start();                   // turn WiFi on
}


// -------------------------
// NTP TIME INIT
// -------------------------
void init_time() {
    sntp_setoperatingmode(SNTP_OPMODE_POLL);  // periodic time sync
    sntp_setservername(0, "pool.ntp.org");    // NTP server
    sntp_init();                              // start SNTP

    // Loop until time is valid
    time_t now = 0;
    struct tm timeinfo = { 0 };

    ESP_LOGI("TIME", "Waiting for NTP time sync...");

    while (timeinfo.tm_year < (2020 - 1900)) {  // before year 2020 = invalid
        vTaskDelay(pdMS_TO_TICKS(200));
        time(&now);
        localtime_r(&now, &timeinfo);
    }

    ESP_LOGI("TIME", "Time synchronized!");
}

// -------------------------
// MAIN APPLICATION
// -------------------------
void app_main(void) {

    // --- Initialize NVS ---
    // Needed for WiFi & ESP-NOW; ensures flash is ready.
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // --- Initialize WiFi ---
    init_wifi();

    // --- Initialize Time via NTP ---
    init_time();

    // --- Configure ADC ---
    adc1_config_width(ADC_WIDTH_BIT_12);                 // 0-4095 resolution
    adc1_config_channel_atten(LE250_PIN, ADC_ATTEN_DB_12); // wider voltage range

    // --- ESP-NOW Init ---
    if (esp_now_init() != ESP_OK) {
        ESP_LOGE(TAG, "ESP-NOW init failed");
        return;
    }

    esp_now_register_send_cb(send_cb);  // register callback

    // --- ESP-NOW Peer Configuration (Broadcast) ---
    esp_now_peer_info_t peerInfo = {0};

    memcpy(peerInfo.peer_addr, broadcast_mac, ESP_NOW_ETH_ALEN); // FF:FF:FF...

    uint8_t channel;
    wifi_second_chan_t second;
    esp_wifi_get_channel(&channel, &second); // get current WiFi channel
    peerInfo.channel = channel;              // match current WiFi channel
    peerInfo.encrypt = false;                // no encryption for broadcast

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add peer");
        return;
    }

    // -------------------------
    // MAIN LOOP
    // -------------------------
    while (1) {
        // 1) Read analog sensor value
        int sensor_val = read_sensor();

        // 2) Create JSON payload
        char* json_str = create_sensor_json("1", sensor_val);

        // 3) Send JSON via ESP-NOW
        esp_err_t result =
            esp_now_send(broadcast_mac, (uint8_t*)json_str, strlen(json_str));

        if (result == ESP_OK) {
            ESP_LOGI(TAG, "Sent JSON: %s", json_str);
        } else {
            ESP_LOGE(TAG, "Send error: %d", result);
        }

        // 4) Free JSON string memory
        free(json_str);

        // 5) Wait 1 second
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
