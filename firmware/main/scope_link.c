#include "scope_link.h"

#include <stdatomic.h>
#include <string.h>

#include "driver/gpio.h"
#include "driver/spi_slave.h"
#include "esp_check.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "adc_capture.h"
#include "board_config.h"
#include "scope_proto.h"

static const char *TAG = "scope_link";

#define SPI_HOST SPI2_HOST

/* Configuración (solo la modifica la tarea SPI, y nunca mientras hay captura en curso). */
static uint8_t cfg_rate_idx = 1;
static uint16_t cfg_window = SCOPE_DEFAULT_WINDOW;
static bool cfg_test_mode;

/* Estado compartido entre la tarea SPI y la tarea de captura. */
static _Atomic uint8_t state = SCOPE_ST_IDLE;
static volatile bool abort_capture;
static bool last_overrun;
static uint8_t last_err_flags;
static uint16_t seq;

static uint16_t *cap_a;
static uint16_t *cap_b;
static uint8_t *tx_buf;
static uint8_t *rx_buf;
static TaskHandle_t capture_task;

static void IRAM_ATTR post_setup_cb(spi_slave_transaction_t *trans)
{
    gpio_set_level(PIN_HANDSHAKE, 1);
}

static void IRAM_ATTR post_trans_cb(spi_slave_transaction_t *trans)
{
    gpio_set_level(PIN_HANDSHAKE, 0);
}

static void set_state(scope_state_t s)
{
    atomic_store(&state, (uint8_t)s);
}

/* Prepara la respuesta: cabecera de estado y, si hay captura lista, los datos. */
static void prepare_tx(void)
{
    uint8_t st = atomic_load(&state);
    scope_header_t h = {
        .state = st,
        .flags = last_err_flags | (last_overrun ? SCOPE_FLAG_OVERRUN : 0) |
                 (cfg_test_mode ? SCOPE_FLAG_TEST_MODE : 0),
        .rate_idx = cfg_rate_idx,
        .window = cfg_window,
        .seq = seq,
        .fs_hz = scope_rate_get(cfg_rate_idx)->fs_hz,
        .n_channels = SCOPE_NUM_CHANNELS,
        .bits = SCOPE_ADC_BITS,
        .fw_version = SCOPE_PROTO_VERSION,
    };
    scope_header_pack(tx_buf, &h);

    size_t bytes = (size_t)cfg_window * sizeof(uint16_t);
    if (st == SCOPE_ST_READY) {
        memcpy(tx_buf + SCOPE_HDR_LEN, cap_a, bytes);
        memcpy(tx_buf + SCOPE_HDR_LEN + bytes, cap_b, bytes);
    } else {
        memset(tx_buf + SCOPE_HDR_LEN, 0, bytes * SCOPE_NUM_CHANNELS);
    }
}

static void reject(void)
{
    last_err_flags |= SCOPE_FLAG_CMD_REJECTED;
}

static void handle_command(const uint8_t *rx, size_t len)
{
    scope_cmd_frame_t f;

    if (len < SCOPE_CMD_LEN) {
        return;
    }
    if (!scope_cmd_decode(rx, len, &f)) {
        last_err_flags = SCOPE_FLAG_BAD_CMD_CHK;
        return;
    }
    if (f.cmd == SCOPE_CMD_NOP) {
        return;
    }

    uint8_t st = atomic_load(&state);
    bool busy = (st == SCOPE_ST_ARMED);
    last_err_flags = 0;

    switch (f.cmd) {
    case SCOPE_CMD_SET_RATE:
        if (busy || scope_rate_get((uint8_t)f.arg) == NULL) {
            reject();
        } else {
            cfg_rate_idx = (uint8_t)f.arg;
            set_state(SCOPE_ST_IDLE);
        }
        break;
    case SCOPE_CMD_SET_WINDOW:
        if (busy || !scope_window_valid(f.arg)) {
            reject();
        } else {
            cfg_window = f.arg;
            set_state(SCOPE_ST_IDLE);
        }
        break;
    case SCOPE_CMD_SET_TESTMODE:
        if (busy || f.arg > 1) {
            reject();
        } else {
            cfg_test_mode = f.arg == 1;
            set_state(SCOPE_ST_IDLE);
        }
        break;
    case SCOPE_CMD_ARM:
        if (busy) {
            reject();
        } else {
            abort_capture = false;
            last_overrun = false;
            set_state(SCOPE_ST_ARMED);
            xTaskNotifyGive(capture_task);
        }
        break;
    case SCOPE_CMD_RESET:
        if (busy) {
            abort_capture = true;
        } else {
            set_state(SCOPE_ST_IDLE);
        }
        break;
    default:
        reject();
        break;
    }
}

static void spi_task(void *arg)
{
    for (;;) {
        prepare_tx();
        memset(rx_buf, 0, SCOPE_MAX_FRAME);

        spi_slave_transaction_t t = {
            .length = SCOPE_MAX_FRAME * 8,
            .tx_buffer = tx_buf,
            .rx_buffer = rx_buf,
        };
        if (spi_slave_transmit(SPI_HOST, &t, portMAX_DELAY) == ESP_OK) {
            handle_command(rx_buf, t.trans_len / 8);
        }
    }
}

static void capture_task_fn(void *arg)
{
    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        const scope_rate_t *rate = scope_rate_get(cfg_rate_idx);
        esp_err_t err;
        bool overrun = false;

        if (cfg_test_mode) {
            capture_fill_test_pattern(cfg_window, cap_a, cap_b);
            err = ESP_OK;
        } else {
            err = capture_run(rate, cfg_window, cap_a, cap_b, &overrun, &abort_capture);
        }

        last_overrun = overrun;
        if (err == ESP_OK) {
            seq++;
            set_state(SCOPE_ST_READY);
        } else if (err == ESP_ERR_INVALID_STATE) {
            set_state(SCOPE_ST_IDLE);
        } else {
            set_state(SCOPE_ST_ERROR);
        }
    }
}

esp_err_t scope_link_start(void)
{
    cap_a = heap_caps_calloc(SCOPE_MAX_WINDOW, sizeof(uint16_t), MALLOC_CAP_INTERNAL);
    cap_b = heap_caps_calloc(SCOPE_MAX_WINDOW, sizeof(uint16_t), MALLOC_CAP_INTERNAL);
    tx_buf = heap_caps_calloc(1, SCOPE_MAX_FRAME, MALLOC_CAP_DMA);
    rx_buf = heap_caps_calloc(1, SCOPE_MAX_FRAME, MALLOC_CAP_DMA);
    ESP_RETURN_ON_FALSE(cap_a && cap_b && tx_buf && rx_buf, ESP_ERR_NO_MEM, TAG, "sin memoria");

    gpio_config_t hs = {
        .pin_bit_mask = 1ULL << PIN_HANDSHAKE,
        .mode = GPIO_MODE_OUTPUT,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&hs), TAG, "gpio handshake");
    gpio_set_level(PIN_HANDSHAKE, 0);

    spi_bus_config_t bus = {
        .mosi_io_num = PIN_SPI_MOSI,
        .miso_io_num = PIN_SPI_MISO,
        .sclk_io_num = PIN_SPI_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = SCOPE_MAX_FRAME,
    };
    spi_slave_interface_config_t slv = {
        .mode = 0,
        .spics_io_num = PIN_SPI_CS,
        .queue_size = 1,
        .flags = 0,
        .post_setup_cb = post_setup_cb,
        .post_trans_cb = post_trans_cb,
    };
    ESP_RETURN_ON_ERROR(spi_slave_initialize(SPI_HOST, &bus, &slv, SPI_DMA_CH_AUTO), TAG, "spi init");

    xTaskCreate(capture_task_fn, "capture", 4096, NULL, 6, &capture_task);
    xTaskCreate(spi_task, "spi_link", 4096, NULL, 7, NULL);

    ESP_LOGI(TAG, "SPI esclavo listo (SCLK=%d MOSI=%d MISO=%d CS=%d HS=%d)",
             PIN_SPI_SCLK, PIN_SPI_MOSI, PIN_SPI_MISO, PIN_SPI_CS, PIN_HANDSHAKE);
    return ESP_OK;
}
