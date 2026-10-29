#include "esp_log.h"

#include "scope_link.h"

static const char *TAG = "main";

/*
 * Punto de entrada. Toda la lógica corre en dos tareas FreeRTOS creadas por
 * scope_link_start(); app_main solo inicializa y termina.
 */
void app_main(void)
{
    ESP_LOGI(TAG, "Osciloscopio ESP32-C6: esclavo SPI + ADC continuo");
    ESP_ERROR_CHECK(scope_link_start());
}
