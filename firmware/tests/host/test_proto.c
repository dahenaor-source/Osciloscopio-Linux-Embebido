/* Pruebas unitarias del protocolo en el PC: `make -C firmware/tests/host test` */
#include <stdio.h>
#include <string.h>

#include "scope_proto.h"

static int failures;

#define CHECK(cond)                                                      \
    do {                                                                 \
        if (!(cond)) {                                                   \
            printf("FALLO %s:%d: %s\n", __FILE__, __LINE__, #cond);      \
            failures++;                                                  \
        }                                                                \
    } while (0)

static void test_cmd_roundtrip(void)
{
    uint8_t buf[SCOPE_CMD_LEN];
    scope_cmd_frame_t f;

    scope_cmd_encode(buf, SCOPE_CMD_SET_WINDOW, 1020);
    CHECK(scope_cmd_decode(buf, sizeof buf, &f));
    CHECK(f.cmd == SCOPE_CMD_SET_WINDOW);
    CHECK(f.arg == 1020);
}

static void test_cmd_nop_is_valid(void)
{
    uint8_t buf[SCOPE_CMD_LEN];
    scope_cmd_frame_t f;

    scope_cmd_encode(buf, SCOPE_CMD_NOP, 0);
    CHECK(buf[3] == SCOPE_CMD_CHK_SEED);
    CHECK(scope_cmd_decode(buf, sizeof buf, &f));
}

static void test_cmd_rejects_corruption(void)
{
    uint8_t buf[SCOPE_CMD_LEN];
    scope_cmd_frame_t f;

    scope_cmd_encode(buf, SCOPE_CMD_ARM, 0);
    for (int i = 0; i < SCOPE_CMD_LEN; i++) {
        uint8_t bad[SCOPE_CMD_LEN];
        memcpy(bad, buf, sizeof bad);
        bad[i] ^= 0x01;
        CHECK(!scope_cmd_decode(bad, sizeof bad, &f));
    }
    CHECK(!scope_cmd_decode(buf, SCOPE_CMD_LEN - 1, &f));
}

static void test_header_roundtrip(void)
{
    scope_header_t in = {.state = SCOPE_ST_READY, .flags = SCOPE_FLAG_OVERRUN,
                         .rate_idx = 3, .window = 1020, .seq = 0xABCD,
                         .fs_hz = 40000, .n_channels = 2, .bits = 12,
                         .fw_version = SCOPE_PROTO_VERSION};
    scope_header_t out;
    uint8_t buf[SCOPE_HDR_LEN];

    scope_header_pack(buf, &in);
    CHECK(buf[0] == SCOPE_HDR_MAGIC);
    CHECK(scope_header_unpack(buf, sizeof buf, &out));
    CHECK(out.state == in.state && out.flags == in.flags);
    CHECK(out.rate_idx == in.rate_idx && out.window == in.window);
    CHECK(out.seq == in.seq && out.fs_hz == in.fs_hz);
    CHECK(out.n_channels == in.n_channels && out.bits == in.bits);
    CHECK(out.fw_version == in.fw_version);
}

static void test_header_rejects_corruption(void)
{
    scope_header_t h = {.state = SCOPE_ST_IDLE, .window = 256, .fs_hz = 1000,
                        .n_channels = 2, .bits = 12};
    scope_header_t out;
    uint8_t buf[SCOPE_HDR_LEN];

    scope_header_pack(buf, &h);
    for (int i = 0; i < SCOPE_HDR_LEN; i++) {
        uint8_t bad[SCOPE_HDR_LEN];
        memcpy(bad, buf, sizeof bad);
        bad[i] ^= 0x80;
        CHECK(!scope_header_unpack(bad, sizeof bad, &out));
    }
    CHECK(!scope_header_unpack(buf, SCOPE_HDR_LEN - 1, &out));
}

static void test_rates(void)
{
    /* Requisito: presets de 100, 1000 y 10000 Sa/s, y 40000 para la bonificación. */
    CHECK(scope_rate_get(0)->fs_hz == 100);
    CHECK(scope_rate_get(1)->fs_hz == 1000);
    CHECK(scope_rate_get(2)->fs_hz == 10000);
    CHECK(scope_rate_get(3)->fs_hz == 40000);
    CHECK(scope_rate_get(SCOPE_RATE_COUNT) == NULL);

    for (int i = 0; i < SCOPE_RATE_COUNT; i++) {
        const scope_rate_t *r = scope_rate_get((uint8_t)i);
        /* Límites del ADC continuo del ESP32-C6. */
        CHECK(r->adc_total_hz >= 611 && r->adc_total_hz <= 83333);
        CHECK(r->adc_total_hz == r->fs_hz * SCOPE_NUM_CHANNELS * r->decim);
    }
}

static void test_window(void)
{
    CHECK(!scope_window_valid(63));
    CHECK(scope_window_valid(64));
    CHECK(scope_window_valid(SCOPE_MAX_WINDOW));
    CHECK(!scope_window_valid(SCOPE_MAX_WINDOW + 1));
    CHECK(scope_frame_len(SCOPE_MAX_WINDOW) == SCOPE_MAX_FRAME);
    CHECK(SCOPE_MAX_FRAME == 4096);
}

int main(void)
{
    test_cmd_roundtrip();
    test_cmd_nop_is_valid();
    test_cmd_rejects_corruption();
    test_header_roundtrip();
    test_header_rejects_corruption();
    test_rates();
    test_window();

    if (failures) {
        printf("%d fallos\n", failures);
        return 1;
    }
    printf("OK: todas las pruebas del protocolo pasaron\n");
    return 0;
}
