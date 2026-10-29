#include "esp_log.h"

#include "scope_link.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "Osciloscopio ESP32-C6: esclavo SPI + ADC continuo");
    ESP_ERROR_CHECK(scope_link_start());
}
