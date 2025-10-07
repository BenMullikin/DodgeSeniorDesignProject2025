#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

void app_main(void) {
    while (1) {
        ESP_LOGI("SENSOR", "Hello from sensor!");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}