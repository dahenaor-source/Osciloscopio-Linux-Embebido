#include "scope_proto.h"

/*
 * El ADC del ESP32-C6 solo acepta entre 611 Sa/s y 83.333 Sa/s en total
 * (todos los canales). Para 100 Sa/s por canal el ADC corre a 2 kSa/s en total
 * y se promedian 10 conversiones por muestra.
 */
static const scope_rate_t k_rates[SCOPE_RATE_COUNT] = {
    {.fs_hz = 100, .adc_total_hz = 2000, .decim = 10},
    {.fs_hz = 1000, .adc_total_hz = 2000, .decim = 1},
    {.fs_hz = 10000, .adc_total_hz = 20000, .decim = 1},
    {.fs_hz = 40000, .adc_total_hz = 80000, .decim = 1},
};

const scope_rate_t *scope_rate_get(uint8_t idx)
{
    return idx < SCOPE_RATE_COUNT ? &k_rates[idx] : NULL;
}

bool scope_window_valid(uint16_t window)
{
    return window >= SCOPE_MIN_WINDOW && window <= SCOPE_MAX_WINDOW;
}

size_t scope_frame_len(uint16_t window)
{
    return SCOPE_HDR_LEN + (size_t)SCOPE_NUM_CHANNELS * window * 2;
}

/* Comando: [cmd, arg_lo, arg_hi, chk]. El checksum lleva una semilla para que un bus en 0x00 no parezca un NOP válido. */
static uint8_t cmd_chk(uint8_t cmd, uint8_t lo, uint8_t hi)
{
    return (uint8_t)(cmd ^ lo ^ hi ^ SCOPE_CMD_CHK_SEED);
}

void scope_cmd_encode(uint8_t out[SCOPE_CMD_LEN], uint8_t cmd, uint16_t arg)
{
    out[0] = cmd;
    out[1] = (uint8_t)(arg & 0xFF);
    out[2] = (uint8_t)(arg >> 8);
    out[3] = cmd_chk(out[0], out[1], out[2]);
}

bool scope_cmd_decode(const uint8_t *buf, size_t len, scope_cmd_frame_t *out)
{
    if (len < SCOPE_CMD_LEN || buf[3] != cmd_chk(buf[0], buf[1], buf[2])) {
        return false;
    }
    out->cmd = buf[0];
    out->arg = (uint16_t)(buf[1] | (buf[2] << 8));
    return true;
}

static uint8_t xor_bytes(const uint8_t *p, size_t n)
{
    uint8_t x = 0;
    for (size_t i = 0; i < n; i++) {
        x ^= p[i];
    }
    return x;
}

/* Cabecera de 16 bytes: magic + estado + parámetros + checksum XOR. Detecta lecturas hechas con el esclavo no listo. */
void scope_header_pack(uint8_t out[SCOPE_HDR_LEN], const scope_header_t *h)
{
    out[0] = SCOPE_HDR_MAGIC;
    out[1] = h->state;
    out[2] = h->flags;
    out[3] = h->rate_idx;
    out[4] = (uint8_t)(h->window & 0xFF);
    out[5] = (uint8_t)(h->window >> 8);
    out[6] = (uint8_t)(h->seq & 0xFF);
    out[7] = (uint8_t)(h->seq >> 8);
    out[8] = (uint8_t)(h->fs_hz & 0xFF);
    out[9] = (uint8_t)((h->fs_hz >> 8) & 0xFF);
    out[10] = (uint8_t)((h->fs_hz >> 16) & 0xFF);
    out[11] = (uint8_t)((h->fs_hz >> 24) & 0xFF);
    out[12] = h->n_channels;
    out[13] = h->bits;
    out[14] = h->fw_version;
    out[15] = xor_bytes(out, SCOPE_HDR_LEN - 1);
}

bool scope_header_unpack(const uint8_t *buf, size_t len, scope_header_t *out)
{
    if (len < SCOPE_HDR_LEN || buf[0] != SCOPE_HDR_MAGIC ||
        buf[15] != xor_bytes(buf, SCOPE_HDR_LEN - 1)) {
        return false;
    }
    out->state = buf[1];
    out->flags = buf[2];
    out->rate_idx = buf[3];
    out->window = (uint16_t)(buf[4] | (buf[5] << 8));
    out->seq = (uint16_t)(buf[6] | (buf[7] << 8));
    out->fs_hz = (uint32_t)buf[8] | ((uint32_t)buf[9] << 8) |
                 ((uint32_t)buf[10] << 16) | ((uint32_t)buf[11] << 24);
    out->n_channels = buf[12];
    out->bits = buf[13];
    out->fw_version = buf[14];
    return true;
}
