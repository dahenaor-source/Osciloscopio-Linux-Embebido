# 11. Guía rápida: flashear y probar el ESP32-C6

El firmware (v2) está compilado y las pruebas del protocolo pasan; la v1 se probó con la BeaglePlay y la v2 está pendiente de flashear y medir. **No hace falta instalar ESP-IDF**: basta flashear el binario de [`release/`](../../release) y conectar el SBC.

## 1. Qué necesitas

| Elemento | Para qué |
|---|---|
| ESP32-C6-DevKitC-1 + cable USB-C (puerto **UART/USB**) | Alimentar y flashear |
| BeaglePlay con su SPI de mikroBUS accesible como `/dev/spidevX.Y` | Maestro SPI |
| 5 cables dupont macho-hembra (el mikroBUS de la BeaglePlay tiene orificios; los pines del ESP32-C6 son machos) | SCK, MISO, MOSI, CS y GND (INT opcional) |
| Generador de señales (opcional) | Probar el ADC; máximo 0 a 3,1 V, solo positivo |
| Analizador lógico (opcional) | Verificar reloj SPI y handshake |

Conexiones: ver [03-hardware-y-conexiones.md](03-hardware-y-conexiones.md). Resumen: SCK `GPIO6`, MISO `GPIO2`, MOSI `GPIO7`, CS `GPIO10`, INT `GPIO3`, GND común. **No conectar 3,3 V ni 5 V entre las placas.**

## 2. Flashear el binario (sin ESP-IDF)

Solo Python 3 y `esptool`:

```bash
pip install esptool
esptool.py --chip esp32c6 -p <PUERTO> -b 460800 write_flash 0x0 release/osciloscopio_c6_v2.bin
```

| Sistema | `<PUERTO>` |
|---|---|
| Linux | `/dev/ttyACM0` |
| WSL | `/dev/ttyACM0` (antes hacer `usbipd attach`, ver [05-firmware-esp32c6.md](05-firmware-esp32c6.md)) |
| Windows | `COMx` (ver Administrador de dispositivos) |

- Se flashea en el offset `0x0` (el binario ya incluye bootloader, tabla de particiones y aplicación).
- Si no conecta: mantener pulsado **BOOT**, pulsar y soltar **RESET** y soltar BOOT; reintentar.
- SHA-256 de `osciloscopio_c6_v2.bin`: `1c51a29df9c500cb06603873d2eb6444d1340b4ed6fb0c01d0ccdb8b76a5ac9c`.

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
3. Implementar el protocolo de [04-protocolo-spi.md](04-protocolo-spi.md). `tools/spi_master_test.py` sirve como referencia ya funcional (funciones `cmd_frame`, `parse_header` y `Scope.capture`).
4. Elegir la ventana para cumplir el refresco del dashboard (entre 1 y 4 s por gráfica): a 100 Sa/s, 256 muestras tardan 2,56 s.

## 4.1 Notas de las pruebas con la BeaglePlay

- El **modo de prueba persiste** hasta reiniciar el ESP32-C6 (botón RESET) o enviar `SET_TESTMODE 0`. Si el dashboard muestra «modo test» y dos líneas casi planas (≈ 0 V y ≈ 3,1 V), es ese modo, no el ADC.
- Usar **2 MHz** como velocidad SPI; a 100 kHz se pierden más transacciones.
- Con el firmware v2 las muestras van en **mV**; con la v1, en código crudo. Backend y dashboard deben ser de la misma versión.
- Para ver errores del ADC o del SPI esclavo, abrir el monitor serie (el USB debe estar conectado al PC o a WSL con `usbipd attach`).

## 4.2 Firmware de diagnóstico del enlace SPI

`release/osciloscopio_c6_v2_diag.bin` (SHA-256 `ed18d2f32820e38aa03204ddcff11fd08362ee67348ca31d572707c30d8d3f51`) es el firmware v2 con un contador de transacciones. Se flashea igual que el normal. Solo sirve para averiguar por qué la BeaglePlay no recibe respuesta. En el monitor serie imprime:

- las 10 primeras transacciones recibidas: `diag: transaccion #N len=16 B rx[0..3]=00 00 00 5A`;
- cada 2 s: `diag: trans=N cmd_chk_malo=M len=min..max | niveles CS=a SCK=b MOSI=c | estado=e`.

| Lo que se ve | Significa |
|---|---|
| `trans=0` siempre y `CS=1` | La BeaglePlay **no llega** al ESP32-C6: revisar CS, SCK y GND |
| `trans=0` y `CS=0` fijo | CS está a GND (cable mal puesto o cruzado) |
| `trans` crece y `rx[0..3]=00 00 00 5A` | El ESP32-C6 **sí recibe** el comando; el fallo está en el camino de vuelta (MISO) |
| `trans` crece, `rx` son `FF` o `00` | El ESP32-C6 recibe basura: MOSI o SCK mal conectados, o la velocidad es alta |
| `cmd_chk_malo` crece | Comandos corruptos que el firmware descarta |

Se activa en el código fuente (`diag_task` en `firmware/main/scope_link.c`) con una carga de CPU mínima.

## 5. Si algo falla

| Síntoma | Causa probable |
|---|---|
| `cabecera inválida (magic)` con todo 0x00 o 0xFF | Cableado de MISO/CS o masa común; modo SPI distinto de 0 |
| Funciona a 1 MHz y falla a más | Cables largos, falta de GND común; bajar la velocidad |
| `comando rechazado` | Se envió `ARM` con una captura en curso, o un valor de configuración fuera de rango; enviar `RESET` primero |
| `captura fallida, estado=3` | El ADC no entregó datos a tiempo; revisar con `--test-mode` si el SPI está bien |
| Valores siempre 0 mV o ~3100 mV | Señal fuera de 0 a 3,1 V o sin GND común con el generador |

Cualquier otro maestro SPI sirve para probar el firmware (Raspberry Pi, adaptador USB-SPI como FT232H), mientras use 3,3 V.
