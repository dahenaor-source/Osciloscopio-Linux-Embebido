/*
 * Protocolo SPI del osciloscopio (ESP32-C6 esclavo <-> SBC maestro).
 * Sin dependencias de ESP-IDF: se puede compilar y probar en el PC.
 * Especificación completa: docs/protocolo-spi.md
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SCOPE_PROTO_VERSION 1

#define SCOPE_NUM_CHANNELS 2
#define SCOPE_ADC_BITS 12

#define SCOPE_CMD_LEN 4
#define SCOPE_CMD_CHK_SEED 0x5A

#define SCOPE_HDR_LEN 16
#define SCOPE_HDR_MAGIC 0xE5

#define SCOPE_MIN_WINDOW 64
#define SCOPE_DEFAULT_WINDOW 256
/* 16 + 2 canales * 1020 muestras * 2 bytes = 4096 = bufsiz por defecto de spidev */
#define SCOPE_MAX_WINDOW 1020
#define SCOPE_MAX_FRAME (SCOPE_HDR_LEN + SCOPE_NUM_CHANNELS * SCOPE_MAX_WINDOW * 2)

typedef enum {
    SCOPE_CMD_NOP = 0x00,
    SCOPE_CMD_SET_RATE = 0x01,
    SCOPE_CMD_SET_WINDOW = 0x02,
    SCOPE_CMD_ARM = 0x03,
    SCOPE_CMD_RESET = 0x04,
    SCOPE_CMD_SET_TESTMODE = 0x05,
} scope_cmd_t;

typedef enum {
    SCOPE_ST_IDLE = 0,
    SCOPE_ST_ARMED = 1,
    SCOPE_ST_READY = 2,
    SCOPE_ST_ERROR = 3,
} scope_state_t;

#define SCOPE_FLAG_OVERRUN 0x01
#define SCOPE_FLAG_CMD_REJECTED 0x02
#define SCOPE_FLAG_BAD_CMD_CHK 0x04
#define SCOPE_FLAG_TEST_MODE 0x08

typedef struct {
    uint32_t fs_hz;       /* frecuencia de muestreo por canal */
    uint32_t adc_total_hz; /* frecuencia de conversión del ADC (todos los canales) */
    uint8_t decim;        /* factor de promediado/decimación */
} scope_rate_t;

#define SCOPE_RATE_COUNT 4

typedef struct {
    uint8_t cmd;
    uint16_t arg;
} scope_cmd_frame_t;

typedef struct {
    uint8_t state;
    uint8_t flags;
    uint8_t rate_idx;
    uint16_t window;
    uint16_t seq;
    uint32_t fs_hz;
    uint8_t n_channels;
    uint8_t bits;
    uint8_t fw_version;
} scope_header_t;

const scope_rate_t *scope_rate_get(uint8_t idx);
bool scope_window_valid(uint16_t window);

void scope_cmd_encode(uint8_t out[SCOPE_CMD_LEN], uint8_t cmd, uint16_t arg);
bool scope_cmd_decode(const uint8_t *buf, size_t len, scope_cmd_frame_t *out);

void scope_header_pack(uint8_t out[SCOPE_HDR_LEN], const scope_header_t *h);
bool scope_header_unpack(const uint8_t *buf, size_t len, scope_header_t *out);

/* Tamaño total de la trama de lectura (cabecera + datos) para una ventana. */
size_t scope_frame_len(uint16_t window);
