#include "adc_capture.h"

#include <string.h>

#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_continuous.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "soc/soc_caps.h"

#include "board_config.h"

static const char *TAG = "capture";

#define FRAME_BYTES 1024
#define POOL_BYTES 16384
/* Conversiones iniciales que se descartan mientras el ADC se estabiliza. */
#define SETTLE_CONVERSIONS 16

/*
 * Captura con el ADC1 en modo continuo: el hardware temporiza las conversiones
 * (frecuencia constante) y las entrega por DMA; la CPU solo las clasifica por
 * canal. En modo TYPE2 cada resultado de 2 bytes trae el canal y el dato de 12 bits.
 * El ADC convierte los 2 canales de forma alternada (rate->adc_total_hz en total).
 */
typedef struct {
    uint16_t *out;
    const uint16_t *lut; /* código crudo de 12 bits -> milivoltios */
    uint16_t count;
    uint32_t sum;
    uint8_t acc;
} chan_state_t;

/* Se ejecuta en ISR: el pool del DMA se llenó antes de que lo leyéramos (se perdieron muestras). */
static bool IRAM_ATTR on_pool_overflow(adc_continuous_handle_t handle,
                                       const adc_continuous_evt_data_t *edata, void *user_data)
{
    *(volatile bool *)user_data = true;
    return false;
}

/*
 * Calibración: el ADC del ESP32-C6 no es lineal ni tiene el mismo fondo de escala en cada placa.
 * Se usa el esquema curve-fitting de ESP-IDF (coeficientes grabados en eFuse en fábrica) para
 * construir una tabla de 4096 entradas por canal; así la conversión en la captura es solo una lectura
 * de tabla y no consume CPU. Si la placa no trae datos de calibración se usa una recta aproximada y
 * se informa con el flag SCOPE_FLAG_UNCALIBRATED.
 */
#define FALLBACK_FULL_SCALE_MV 3100

static uint16_t lut_mv[SCOPE_NUM_CHANNELS][4096];
static bool lut_ready;
static bool lut_calibrated;

static void build_lut(void)
{
    static const adc_channel_t chans[SCOPE_NUM_CHANNELS] = {ADC_CH_A, ADC_CH_B};
    lut_calibrated = true;
    for (int c = 0; c < SCOPE_NUM_CHANNELS; c++) {
        adc_cali_handle_t cali = NULL;
        adc_cali_curve_fitting_config_t ccfg = {
            .unit_id = ADC_UNIT_1,
            .chan = chans[c],
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_12,
        };
        esp_err_t e = adc_cali_create_scheme_curve_fitting(&ccfg, &cali);
        for (int raw = 0; raw < 4096; raw++) {
            int mv = 0;
            if (e == ESP_OK && adc_cali_raw_to_voltage(cali, raw, &mv) == ESP_OK) {
                lut_mv[c][raw] = (uint16_t)(mv < 0 ? 0 : mv);
            } else {
                lut_calibrated = false;
                lut_mv[c][raw] = (uint16_t)((raw * FALLBACK_FULL_SCALE_MV) / 4095);
            }
        }
        if (cali) {
            adc_cali_delete_scheme_curve_fitting(cali);
        }
        ESP_LOGI(TAG, "canal %d: calibracion %s", c, e == ESP_OK ? "curve-fitting (eFuse)" : esp_err_to_name(e));
    }
    lut_ready = true;
}

bool capture_is_calibrated(void)
{
    if (!lut_ready) {
        build_lut();
    }
    return lut_calibrated;
}

/* Promedia `decim` conversiones por muestra (decim = 1 -> sin promediar). */
static void push_sample(chan_state_t *c, uint16_t window, uint8_t decim, uint32_t value)
{
    if (c->count >= window) {
        return;
    }
    c->sum += value;
    if (++c->acc == decim) {
        c->out[c->count++] = c->lut[c->sum / decim];
        c->sum = 0;
        c->acc = 0;
    }
}

esp_err_t capture_run(const scope_rate_t *rate, uint16_t window,
                      uint16_t *ch_a, uint16_t *ch_b,
                      bool *overrun, const volatile bool *abort)
{
    adc_continuous_handle_t handle = NULL;
    volatile bool pool_overflow = false;
    esp_err_t err;

    if (!lut_ready) {
        build_lut();
    }

    adc_continuous_handle_cfg_t handle_cfg = {
        .max_store_buf_size = POOL_BYTES, /* pool grande: absorbe 80 kSa/s mientras la CPU atiende el SPI */
        .conv_frame_size = FRAME_BYTES,
    };
    ESP_RETURN_ON_ERROR(adc_continuous_new_handle(&handle_cfg, &handle), TAG, "new_handle");

    adc_digi_pattern_config_t pattern[SCOPE_NUM_CHANNELS] = {
        {.atten = ADC_ATTEN_DB_12, .channel = ADC_CH_A, .unit = ADC_UNIT_1, .bit_width = ADC_BITWIDTH_12},
        {.atten = ADC_ATTEN_DB_12, .channel = ADC_CH_B, .unit = ADC_UNIT_1, .bit_width = ADC_BITWIDTH_12},
    };
    adc_continuous_config_t cfg = {
        .pattern_num = SCOPE_NUM_CHANNELS,
        .adc_pattern = pattern,
        .sample_freq_hz = rate->adc_total_hz,
        .conv_mode = ADC_CONV_SINGLE_UNIT_1,
        .format = ADC_DIGI_OUTPUT_FORMAT_TYPE2,
    };
    err = adc_continuous_config(handle, &cfg);
    if (err != ESP_OK) {
        goto out;
    }

    adc_continuous_evt_cbs_t cbs = {.on_pool_ovf = on_pool_overflow};
    err = adc_continuous_register_event_callbacks(handle, &cbs, (void *)&pool_overflow);
    if (err != ESP_OK) {
        goto out;
    }

    err = adc_continuous_start(handle);
    if (err != ESP_OK) {
        goto out;
    }

    chan_state_t st[SCOPE_NUM_CHANNELS] = {{.out = ch_a, .lut = lut_mv[0]}, {.out = ch_b, .lut = lut_mv[1]}};
    static uint8_t buf[FRAME_BYTES];
    uint32_t settle = SETTLE_CONVERSIONS;
    /* Plazo = duración teórica de la ventana + 1 s de margen, para no bloquearse si el ADC falla. */
    int64_t deadline_us = esp_timer_get_time() +
                          ((int64_t)window * 1000000 / rate->fs_hz) + 1000000;
    err = ESP_OK;

    while (!(st[0].count >= window && st[1].count >= window)) {
        if (*abort) {
            err = ESP_ERR_INVALID_STATE;
            break;
        }
        if (esp_timer_get_time() > deadline_us) {
            err = ESP_ERR_TIMEOUT;
            break;
        }

        uint32_t got = 0;
        esp_err_t r = adc_continuous_read(handle, buf, sizeof buf, &got, 100);
        if (r == ESP_ERR_TIMEOUT) {
            continue;
        }
        if (r != ESP_OK) {
            err = r;
            break;
        }

        for (uint32_t i = 0; i + SOC_ADC_DIGI_RESULT_BYTES <= got; i += SOC_ADC_DIGI_RESULT_BYTES) {
            const adc_digi_output_data_t *p = (const adc_digi_output_data_t *)&buf[i];
            if (settle) {
                settle--;
                continue;
            }
            if (p->type2.channel == ADC_CH_A) {
                push_sample(&st[0], window, rate->decim, p->type2.data);
            } else if (p->type2.channel == ADC_CH_B) {
                push_sample(&st[1], window, rate->decim, p->type2.data);
            }
        }
    }

    adc_continuous_stop(handle);
    *overrun = pool_overflow;

out:
    adc_continuous_deinit(handle);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "captura fallida: %s", esp_err_to_name(err));
    }
    return err;
}

void capture_fill_test_pattern(uint16_t window, uint16_t *ch_a, uint16_t *ch_b)
{
    for (uint16_t i = 0; i < window; i++) {
        ch_a[i] = i & 0x0FFF;
        ch_b[i] = 0x0FFF - (i & 0x0FFF);
    }
}
