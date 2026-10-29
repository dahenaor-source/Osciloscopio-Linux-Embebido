# Guía rápida para el equipo: probar el ESP32-C6

El firmware ya está compilado y verificado. **No hace falta instalar ESP-IDF**: basta flashear el binario de [`release/`](../release) y conectar el SBC.

## 1. Qué necesitas

| Elemento | Para qué |
|---|---|
| ESP32-C6-DevKitC-1 + cable USB-C (puerto **UART/USB**) | Alimentar y flashear |
| BeaglePlay con su SPI de mikroBUS accesible como `/dev/spidevX.Y` | Maestro SPI |
| 5 cables dupont hembra-hembra | SCK, MISO, MOSI, CS y GND (INT opcional) |
| Generador de señales (opcional) | Probar el ADC; máximo 0 a 3,1 V, solo positivo |
| Analizador lógico (opcional) | Verificar reloj SPI y handshake |

Conexiones: ver [hardware-pinout.md](hardware-pinout.md). Resumen: SCK `GPIO6`, MISO `GPIO2`, MOSI `GPIO7`, CS `GPIO10`, INT `GPIO3`, GND común. **No conectar 3,3 V ni 5 V entre las placas.**

## 2. Flashear el binario (sin ESP-IDF)

Solo Python 3 y `esptool`:

```bash
pip install esptool
esptool.py --chip esp32c6 -p <PUERTO> -b 460800 write_flash 0x0 release/osciloscopio_c6_v1.bin
```

| Sistema | `<PUERTO>` |
|---|---|
| Linux | `/dev/ttyACM0` |
| WSL | `/dev/ttyACM0` (antes hacer `usbipd attach`, ver [firmware.md](firmware.md)) |
| Windows | `COMx` (ver Administrador de dispositivos) |

- Se flashea en el offset `0x0` (el binario ya incluye bootloader, tabla de particiones y aplicación).
- Si no conecta: mantener pulsado **BOOT**, pulsar y soltar **RESET** y soltar BOOT; reintentar.
- SHA-256 de `osciloscopio_c6_v1.bin`: `f5f793ba5b343073d21426c9ae5733cac80c51bb8b017f42e009a60cea5baa00`.

Comprobación: abrir el monitor serie a 115200 baudios (`python -m serial.tools.miniterm <PUERTO> 115200`) y pulsar RESET. Debe aparecer:

```
scope_link: SPI esclavo listo (SCLK=6 MOSI=7 MISO=2 CS=10 HS=3)
```

## 3. Probar el SPI desde el SBC

En el SBC (Linux con `spidev`):

```bash
sudo apt install python3-spidev
python3 tools/spi_master_test.py --dev /dev/spidevX.Y --test-mode
```

Resultado esperado: `Integridad de datos: OK`. En ese modo el ESP32-C6 envía rampas sintéticas (no usa el ADC), así que valida el SPI, el cableado y el protocolo por separado.

Después, con el generador conectado a GPIO0/GPIO1:

```bash
python3 tools/spi_master_test.py --dev /dev/spidevX.Y --rate 1 --window 256     # 1 kSa/s
python3 tools/spi_master_test.py --dev /dev/spidevX.Y --rate 3 --window 256     # 40 kSa/s
python3 tools/spi_master_test.py --dev /dev/spidevX.Y --bench --speed 10000000  # caudal
```

Empezar con `--speed 1000000` (1 MHz) y subir progresivamente.

## 4. Cosas que el equipo del SBC debe resolver

1. **Habilitar el SPI de mikroBUS como `/dev/spidev*`** en la BeaglePlay. Por defecto lo gestiona el driver `mikrobus`; hay que desactivarlo o crear un overlay de device tree. La ruta final no está determinada.
2. Usar **modo SPI 0**, 8 bits, MSB primero, y esperar **≥ 2 ms entre transacciones** (o leer la línea INT con `libgpiod`).
3. Implementar el protocolo de [protocolo-spi.md](protocolo-spi.md). `tools/spi_master_test.py` sirve como referencia ya funcional (funciones `cmd_frame`, `parse_header` y `Scope.capture`).
4. Elegir la ventana para cumplir el refresco del dashboard (entre 1 y 4 s por gráfica): a 100 Sa/s, 256 muestras tardan 2,56 s.

## 5. Si algo falla

| Síntoma | Causa probable |
|---|---|
| `cabecera inválida (magic)` con todo 0x00 o 0xFF | Cableado de MISO/CS o masa común; modo SPI distinto de 0 |
| Funciona a 1 MHz y falla a más | Cables largos, falta de GND común; bajar la velocidad |
| `comando rechazado` | Se cambió configuración durante una captura (`ARMED`); enviar `RESET` primero |
| `captura fallida, estado=3` | El ADC no entregó datos a tiempo; revisar con `--test-mode` si el SPI está bien |
| Valores del ADC siempre 0 o 4095 | Señal fuera de 0 a 3,1 V o sin GND común con el generador |

Cualquier otro maestro SPI sirve para probar el firmware (Raspberry Pi, adaptador USB-SPI como FT232H), mientras use 3,3 V.
