"""Diagnóstico paso a paso de una captura SPI (ARM -> polling -> lectura).

Uso en la BeaglePlay:  sudo python3 diagnostico_captura.py [velocidad_hz] [rate_idx]
Imprime el RX crudo de cada transacción para ver dónde falla capture_frame().
"""
import sys
import time

from spi_driver import SPIDriver

SPEED = int(sys.argv[1]) if len(sys.argv) > 1 else 100000
RATE = int(sys.argv[2]) if len(sys.argv) > 2 else 1
STATES = {0: "IDLE", 1: "ARMED", 2: "READY", 3: "ERROR"}

d = SPIDriver(bus=0, device=0, speed_hz=SPEED)
if not d.open():
    sys.exit("No se pudo abrir el SPI")


def tx(nombre, cmd, arg=0, n=16):
    rx = d._transact(cmd, arg, n)
    h = d._parse_header(rx)
    if h is None:
        print(f"{nombre:10s} INVALIDO  RX[:16]={rx[:16].hex(' ')}")
    else:
        print(f"{nombre:10s} {STATES.get(h['state'], '?'):5s} flags={h['flags']:04b} "
              f"rate={h['rate_idx']} win={h['window']} seq={h['seq']} fs={h['fs_hz']}")
    return rx, h


print(f"--- velocidad {SPEED} Hz, preset {RATE} ---")
tx("NOP", 0)
tx("RESET", 4)
tx("NOP", 0)
tx("SET_RATE", 1, RATE)
tx("NOP", 0)            # el efecto del comando se ve en la transacción siguiente
tx("TESTMODE", 5, 1)
tx("NOP", 0)
tx("ARM", 3)

t0 = time.time()
h = None
for i in range(400):
    time.sleep(0.01)
    _, h = tx(f"poll{i}", 0)
    if h and h["state"] == 2:
        break
print(f"espera hasta READY: {time.time() - t0:.2f} s")

if h and h["state"] == 2:
    n = 16 + 4 * h["window"]
    time.sleep(0.002)
    rx = d._transact(0, 0, n)
    hh = d._parse_header(rx[:16])
    print("lectura completa, cabecera:", "OK" if hh else f"INVALIDA {rx[:16].hex(' ')}")
    if hh:
        import struct
        w = hh["window"]
        v = struct.unpack(f"<{2 * w}H", rx[16:16 + 4 * w])
        ok = all(v[i] == i for i in range(w)) and all(v[w + i] == 4095 - i for i in range(w))
        print("rampas A=i, B=4095-i:", "CORRECTAS" if ok else f"ERRORES  A[:8]={v[:8]} B[:8]={v[w:w + 8]}")
d.close()
