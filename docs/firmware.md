# Firmware del microcontrolador (ESP32-C6)

Responsable: David Henao Rojas.

## Arquitectura

```
 GPIO0/GPIO1 ──► ADC1 continuo (DMA) ──► capture_task ──► cap_a / cap_b
                                                              │
 SBC ══ SPI2 esclavo (DMA) ◄══ spi_link_task ◄── prepare_tx ──┘
```

| Módulo | Archivo | Función |
|---|---|---|
| Protocolo | [`components/scope_proto`](../firmware/components/scope_proto) | Presets, comandos y cabecera. Sin dependencias de ESP-IDF, probado en el PC |
| Captura | [`main/adc_capture.c`](../firmware/main/adc_capture.c) | ADC continuo con 2 canales, promediado para 100 Sa/s, detección de desbordamiento |
| Enlace SPI | [`main/scope_link.c`](../firmware/main/scope_link.c) | SPI esclavo con DMA, máquina de estados `IDLE → ARMED → READY`, handshake |
| Pines | [`main/board_config.h`](../firmware/main/board_config.h) | Asignación de pines |

Decisiones de diseño:

- **Captura bajo demanda (`ARM`)**: el dashboard refresca a ≤ 1 Hz, así que no se transmite en continuo. Cada captura arranca el ADC, llena la ventana y lo detiene.
- **ADC continuo (`adc_continuous`)**: el hardware fija el periodo de conversión, lo que garantiza frecuencia de muestreo constante. Límites del ADC del C6: 611 a 83.333 Sa/s en total (`SOC_ADC_SAMPLE_FREQ_THRES_*` en ESP-IDF 5.4.1). Con 2 canales alternados, el máximo es 41,6 kSa/s por canal; el preset de 40 kSa/s usa 80 kSa/s en total.
- **100 Sa/s**: queda bajo el mínimo del ADC, así que se convierte a 2 kSa/s y se promedian 10 conversiones por muestra (además reduce ruido).
- **Cabecera de estado en toda respuesta**: el esclavo SPI debe preparar la respuesta antes de la transacción, por lo que un comando solo se refleja en la transacción siguiente. La cabecera permite al maestro confirmarlo.
- **Prioridades**: `spi_link` (7) > `capture` (6), para atender el SPI aunque haya captura en curso.
- **Memoria**: 2 buffers de captura (1020 × 2 B cada uno), TX y RX de 4096 B (DMA) y pool del ADC de 16 KiB.

## Instalación del entorno (WSL Ubuntu)

```bash
sudo apt install -y git wget flex bison gperf python3 python3-venv cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0
mkdir -p ~/esp && cd ~/esp
git clone -b v5.4.1 --depth 1 --recursive --shallow-submodules https://github.com/espressif/esp-idf.git
cd esp-idf && ./install.sh esp32c6
```

Cada terminal nueva: `. ~/esp/esp-idf/export.sh`

### Acceso al USB desde WSL (usbipd)

En **PowerShell de Windows (administrador)**, una sola vez `bind` y en cada reconexión `attach`:

```powershell
winget install usbipd
usbipd list                          # buscar "USB JTAG/serial debug unit" (VID:PID 303a:1001)
usbipd bind --busid <BUSID>
usbipd attach --wsl --busid <BUSID>
```

En WSL verificar con `ls /dev/ttyACM*` (en este equipo: `/dev/ttyACM0`). Si no hay permisos: `sudo usermod -aG dialout $USER` y reabrir la terminal. El `attach` se pierde al reiniciar la placa en modo descarga, o al desconectar el USB; hay que repetirlo.

## Compilar y flashear

```bash
. ~/esp/esp-idf/export.sh
cd firmware
idf.py set-target esp32c6        # solo la primera vez
idf.py build
idf.py -p /dev/ttyACM0 flash monitor     # salir del monitor: Ctrl+]
```

Salida esperada en el monitor: `SPI esclavo listo (SCLK=6 MOSI=7 MISO=2 CS=10 HS=3)`.

## Tasas teóricas de transferencia SPI

| Elemento | Máximo | Fuente |
|---|---|---|
| ESP32-C6, SPI2 como esclavo, full-duplex | 40 MHz | Datasheet ESP32-C6 v1.5, §4.2.1.2 |
| BeaglePlay (AM62x), McSPI como maestro | 50 MHz | Ejemplos de TI MCU+ SDK para AM62x; **confirmar con el datasheet/TRM del AM62x** |
| **Límite teórico del enlace** | **40 MHz = 40 Mbit/s = 5 MB/s** | mínimo de los dos |

Demanda real del sistema:

| Preset | Datos por segundo (2 canales × 2 B) | Equivale a |
|---|---|---|
| 10 kSa/s | 40 kB/s | 0,32 Mbit/s |
| 40 kSa/s | 160 kB/s | 1,28 Mbit/s |

Una ventana máxima (4096 B = 32.768 bits) tarda 0,82 ms a 40 MHz, 3,3 ms a 10 MHz y 33 ms a 1 MHz. Para el preset de 40 kSa/s, 256 muestras (1040 B) llegan al SBC en 0,2 ms a 40 MHz o 8 ms a 1 MHz: el SPI nunca es el cuello de botella; el ADC (83,3 kSa/s) sí.

**Pendiente de verificar experimentalmente** (con analizador lógico y `tools/spi_master_test.py --bench`): velocidad máxima estable sin errores de datos, y caudal real medido. El `spidev` de Linux limita el caudal por la sobrecarga del sistema operativo, así que el valor real será menor al teórico.

## Pruebas

### Unitarias en el PC (protocolo)

```bash
make -C firmware/tests/host test
```

### Pruebas en hardware (con la ESP32-C6 y un maestro SPI)

```bash
python3 tools/spi_master_test.py --dev /dev/spidevX.Y --test-mode            # integridad de datos
python3 tools/spi_master_test.py --dev /dev/spidevX.Y --rate 3 --window 256   # captura real
python3 tools/spi_master_test.py --dev /dev/spidevX.Y --bench --speed 10000000
```

## Matriz de verificación del firmware

| ID | Requisito / función | Prueba | Tipo | Estado |
|---|---|---|---|---|
| FW-01 | Codificación y verificación de comandos | `test_cmd_*` | Unitaria (PC) | ✅ Pasa |
| FW-02 | Cabecera de estado con checksum | `test_header_*` | Unitaria (PC) | ✅ Pasa |
| FW-03 | Presets 100/1000/10000/40000 Sa/s dentro de los límites del ADC | `test_rates` | Unitaria (PC) | ✅ Pasa |
| FW-04 | Ventanas de 64 a 1020 muestras (≥ 64) | `test_window` | Unitaria (PC) | ✅ Pasa |
| FW-05 | El firmware compila con ESP-IDF 5.4.1 | `idf.py build` | Compilación | ✅ Pasa |
| FW-06 | Enlace SPI sin errores de datos (modo prueba) | `spi_master_test.py --test-mode` | Integración | ⏳ Pendiente de hardware |
| FW-07 | Cambio de preset por SPI | `spi_master_test.py --rate N` y lectura de `fs_hz` | Integración | ⏳ Pendiente |
| FW-08 | Frecuencia de muestreo real de cada preset | Analizador lógico / señal de frecuencia conocida | Experimental | ⏳ Pendiente |
| FW-09 | Captura a 40 kSa/s sin desbordamiento (bonificación) | `flags` bit0 = 0 y señal conocida | Experimental | ⏳ Pendiente |
| FW-10 | Velocidad máxima de SPI estable y caudal real | `--bench` a distintas velocidades | Experimental | ⏳ Pendiente |
| FW-11 | Exactitud del ADC (0 V, mitad de escala, 3 V) | Voltímetro vs. valor leído | Experimental | ⏳ Pendiente |
| FW-12 | Integración con el SBC y dashboard | Prueba de extremo a extremo | Integración | ⏳ Pendiente |

## Limitaciones conocidas

- El ADC del ESP32-C6 tiene una no linealidad apreciable cerca de 0 V y de plena escala; para medidas precisas, calibrar o limitar el rango útil.
- Con 2 canales sobre un solo ADC, el canal B está desfasado medio periodo de conversión respecto al A.
- Sin disparo (trigger): cada captura comienza al recibir `ARM`.
