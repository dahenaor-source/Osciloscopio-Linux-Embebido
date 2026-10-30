import os
import time
import struct
import threading
from typing import Optional, Dict, Any

MAGIC_HEADER = 0xE5

class SPIDriver:
    def __init__(self, bus: int = 0, device: int = 0, speed_hz: int = 100000):
        self.bus = bus
        self.device = device
        self.speed_hz = speed_hz
        self.spi = None
        self._lock = threading.RLock()
        self.last_error = ""
        self.retries = 0  # transacciones inválidas reintentadas (contador acumulado)

    def open(self) -> bool:
        """Inicializa la interfaz SPI de la BeaglePlay en Modo 0."""
        try:
            import spidev
            self.spi = spidev.SpiDev()
            self.spi.open(self.bus, self.device)
            self.spi.max_speed_hz = self.speed_hz
            self.spi.mode = 0
            
            # Transacción "dummy" inicial para limpiar el buffer
            self.spi.xfer2([0x00] * 16)
            return True
        except Exception as e:
            print(f"[SPI Driver Error] No se pudo abrir la interfaz SPI: {e}")
            self.spi = None
            return False

    def close(self):
        """Cierra la conexión del bus SPI."""
        if self.spi:
            self.spi.close()

    def _build_command(self, cmd: int, arg: int = 0) -> list:
        """Construye la trama de comando de 4 bytes con Checksum XOR."""
        arg_lo = arg & 0xFF
        arg_hi = (arg >> 8) & 0xFF
        checksum = cmd ^ arg_lo ^ arg_hi ^ 0x5A
        return [cmd, arg_lo, arg_hi, checksum]

    def _transact(self, cmd: int, arg: int = 0, read_len: int = 16) -> bytes:
        """Envía comando y lee la respuesta en una transacción Full-Duplex."""
        if not self.spi:
            return b""
        
        tx = self._build_command(cmd, arg)
        if read_len > 4:
            tx += [0x00] * (read_len - 4)
            
        time.sleep(0.002)  # Respetar tiempo de guarda ≥ 2ms según protocolo
        rx = self.spi.xfer2(tx)
        return bytes(rx)

    def _parse_header(self, raw_header: bytes) -> Optional[Dict[str, Any]]:
        """Valida Magic Byte (0xE5) y Checksum XOR de la cabecera de 16 bytes."""
        if len(raw_header) < 16:
            return None

        # Formato Little-Endian (<BBBBHHIBBBB)
        magic, state, flags, rate_idx, window, seq, fs_hz, n_channels, bits, fw_ver, chk = struct.unpack(
            "<BBBBHHIBBBB", raw_header[:16]
        )

        if magic != MAGIC_HEADER:
            return None

        calc_chk = 0
        for b in raw_header[:15]:
            calc_chk ^= b

        if calc_chk != chk:
            return None

        return {
            "magic": magic,
            "state": state,       # 0: IDLE, 1: ARMED, 2: READY, 3: ERROR
            "flags": flags,
            "rate_idx": rate_idx,
            "window": window,
            "seq": seq,
            "fs_hz": fs_hz,
            "n_channels": n_channels,
            "bits": bits,
            "fw_version": fw_ver
        }

    def _xfer_valid(self, cmd: int, arg: int = 0, read_len: int = 16, tries: int = 12):
        """
        Transacción con reintentos hasta recibir una cabecera válida.
        El ESP32-C6 pierde parte de las transacciones (responde 0xFF o datos
        desplazados) y en esos casos recibe un comando corrupto que descarta por
        checksum. Por eso una respuesta válida implica que el comando se ejecutó
        exactamente una vez; reintentar es seguro.
        Devuelve (cabecera, bytes_rx) o (None, b"").
        """
        for _ in range(tries):
            rx = self._transact(cmd, arg, read_len)
            header = self._parse_header(rx)
            if header is not None:
                return header, rx
            self.retries += 1
            time.sleep(0.005)
        return None, b""

    def set_rate(self, rate_idx: int) -> bool:
        """SET_RATE y verificación en la transacción siguiente (la respuesta va un paso atrasada)."""
        with self._lock:
            if self._xfer_valid(0x01, rate_idx)[0] is None:
                return False
            header, _ = self._xfer_valid(0x00)
            return (header is not None and header["rate_idx"] == rate_idx
                    and header["flags"] & 0x02 == 0)

    def capture_frame(self, window: Optional[int] = None) -> Optional[Dict[str, Any]]:
        """
        ARM -> espera sin tráfico SPI -> sondeo con NOP hasta READY -> lectura de 16 + 4*window bytes.
        La ventana se toma de la cabecera del esclavo (window se ignora).
        Si falla devuelve None y deja la causa en self.last_error.
        """
        with self._lock:
            self.last_error = ""
            retries0 = self.retries
            header, _ = self._xfer_valid(0x00)
            if header is None:
                self.last_error = "sin cabecera valida antes de ARM"
                return None
            seq0 = header["seq"]
            fs = max(header["fs_hz"], 1)
            duration = header["window"] / fs

            for attempt in range(4):
                if self._xfer_valid(0x03)[0] is None:  # ARM
                    self.last_error = "ARM sin respuesta valida"
                    return None
                # Sin tráfico SPI mientras el ADC muestrea: el ruido de las líneas SPI se acopla a la entrada analógica.
                time.sleep(duration * 0.9)

                deadline = time.time() + duration + 2.0
                header = None
                while time.time() < deadline:
                    header, _ = self._xfer_valid(0x00)
                    if header is None:
                        self.last_error = "sondeo sin cabecera valida"
                        return None
                    if header["state"] == 3:
                        self.last_error = "el ESP32 reporta ERROR (fallo del ADC)"
                        return None
                    if header["state"] == 2:
                        break
                    time.sleep(0.002)
                if header["state"] == 2 and header["seq"] == seq0:
                    continue  # READY viejo: el ARM se perdio, se reenvia
                break
            if header["state"] != 2 or header["seq"] == seq0:
                self.last_error = f"timeout esperando READY (estado={header['state']})"
                return None

            w = header["window"]
            total = 16 + 4 * w
            header, rx = self._xfer_valid(0x00, 0, total)
            if header is None or header["state"] != 2 or len(rx) < total:
                self.last_error = "lectura de datos invalida"
                return None

            samples = struct.unpack(f"<{2 * w}H", rx[16:total])
            return {
                "seq": header["seq"],
                "rate_idx": header["rate_idx"],
                "fs_hz": header["fs_hz"],
                "window": w,
                # bit2 (checksum de comando) lo provoca una transacción perdida que ya se reintentó
                "flags": header["flags"] & ~0x04,
                "retries": self.retries - retries0,
                "a": list(samples[:w]),
                "b": list(samples[w:]),
            }
