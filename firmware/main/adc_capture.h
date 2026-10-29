#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"
#include "scope_proto.h"

/*
 * Captura una ventana de `window` muestras por canal con el ADC continuo (DMA).
 * Bloquea hasta completar, hasta que *abort sea true o hasta agotar el tiempo.
 * Los datos quedan en ch_a / ch_b (12 bits, 0..4095).
 */
esp_err_t capture_run(const scope_rate_t *rate, uint16_t window,
                      uint16_t *ch_a, uint16_t *ch_b,
                      bool *overrun, const volatile bool *abort);

/* Señal sintética (rampas) para probar el enlace SPI sin depender del ADC. */
void capture_fill_test_pattern(uint16_t window, uint16_t *ch_a, uint16_t *ch_b);
