#!/usr/bin/env python3
"""Maestro SPI de referencia para probar el firmware del ESP32-C6.

Sirve para verificar el enlace desde la BeaglePlay (o cualquier Linux con
spidev) y como ejemplo del protocolo descrito en docs/protocolo-spi.md.

Uso:
  python3 spi_master_test.py --dev /dev/spidev0.0 --test-mode      # integridad
  python3 spi_master_test.py --dev /dev/spidev0.0 --rate 3 --window 256
  python3 spi_master_test.py --dev /dev/spidev0.0 --bench --speed 10000000

Requiere: pip install spidev  (o apt install python3-spidev)
"""
import argparse
import struct
import sys
import time

CMD_NOP, CMD_SET_RATE, CMD_SET_WINDOW, CMD_ARM, CMD_RESET, CMD_SET_TESTMODE = range(6)
ST_IDLE, ST_ARMED, ST_READY, ST_ERROR = range(4)
HDR_LEN = 16
HDR_MAGIC = 0xE5
GAP_S = 0.002  # separación mínima entre transacciones (el esclavo re-arma su DMA)


def cmd_frame(cmd, arg=0):
    lo, hi = arg & 0xFF, (arg >> 8) & 0xFF
    return [cmd, lo, hi, cmd ^ lo ^ hi ^ 0x5A]


def parse_header(rx):
    if len(rx) < HDR_LEN or rx[0] != HDR_MAGIC:
        raise IOError("cabecera inválida (magic): %s" % bytes(rx[:HDR_LEN]).hex())
    chk = 0
    for b in rx[:HDR_LEN - 1]:
        chk ^= b
    if chk != rx[15]:
        raise IOError("cabecera inválida (checksum)")
    state, flags, rate, window, seq, fs, nch, bits, ver = struct.unpack("<BBBHHIBBB", bytes(rx[1:15]))
    return dict(state=state, flags=flags, rate=rate, window=window, seq=seq,
                fs=fs, channels=nch, bits=bits, version=ver)


class Scope:
    def __init__(self, dev, speed):
        import spidev
        bus, cs = [int(x) for x in dev.replace("/dev/spidev", "").split(".")]
        self.spi = spidev.SpiDev()
        self.spi.open(bus, cs)
        self.spi.mode = 0
        self.spi.max_speed_hz = speed
        self.spi.bits_per_word = 8

    def xfer(self, cmd=CMD_NOP, arg=0, nbytes=HDR_LEN):
        tx = cmd_frame(cmd, arg) + [0] * (nbytes - 4)
        rx = self.spi.xfer2(tx)
        time.sleep(GAP_S)
        return rx

    def status(self):
        return parse_header(self.xfer())

    def command(self, cmd, arg=0):
        self.xfer(cmd, arg)
        h = self.status()
        if h["flags"] & 0x06:
            raise IOError("comando %d rechazado, flags=0x%02x" % (cmd, h["flags"]))
        return h

    def capture(self, timeout=15.0):
        self.command(CMD_ARM)
        t0 = time.time()
        while True:
            h = self.status()
            if h["state"] == ST_READY:
                break
            if h["state"] == ST_ERROR or time.time() - t0 > timeout:
                raise IOError("captura fallida, estado=%d" % h["state"])
            time.sleep(0.01)
        n = h["window"]
        rx = self.xfer(CMD_NOP, 0, HDR_LEN + 4 * n)
        h = parse_header(rx)
        data = struct.unpack("<%dH" % (2 * n), bytes(rx[HDR_LEN:HDR_LEN + 4 * n]))
        return h, list(data[:n]), list(data[n:])


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--dev", default="/dev/spidev0.0")
    ap.add_argument("--speed", type=int, default=1_000_000, help="Hz")
    ap.add_argument("--rate", type=int, default=1, help="preset 0..3 = 100/1000/10000/40000 Sa/s")
    ap.add_argument("--window", type=int, default=256)
    ap.add_argument("--test-mode", action="store_true", help="datos sintéticos (rampas) para validar el SPI")
    ap.add_argument("--bench", action="store_true", help="mide el caudal SPI real")
    args = ap.parse_args()

    sc = Scope(args.dev, args.speed)
    h = sc.status()
    print("Estado inicial:", h)

    sc.command(CMD_SET_TESTMODE, 1 if args.test_mode else 0)
    sc.command(CMD_SET_RATE, args.rate)
    sc.command(CMD_SET_WINDOW, args.window)

    h, a, b = sc.capture()
    print("Captura #%d: fs=%d Sa/s, %d muestras/canal, flags=0x%02x" % (h["seq"], h["fs"], h["window"], h["flags"]))
    print("CH A: min=%d max=%d | CH B: min=%d max=%d" % (min(a), max(a), min(b), max(b)))

    if args.test_mode:
        ok = all(a[i] == (i & 0xFFF) and b[i] == 0xFFF - (i & 0xFFF) for i in range(len(a)))
        print("Integridad de datos:", "OK" if ok else "FALLO")
        if not ok:
            sys.exit(1)

    if args.bench:
        n = 200
        nbytes = HDR_LEN + 4 * args.window
        sc.command(CMD_SET_TESTMODE, 1)
        sc.capture()
        t0 = time.perf_counter()
        for _ in range(n):
            sc.spi.xfer2([0] * nbytes)
        dt = time.perf_counter() - t0
        print("Caudal: %.2f MB/s con %d bytes por transacción (incluye pausas del SO)"
              % (n * nbytes / dt / 1e6, nbytes))


if __name__ == "__main__":
    main()
