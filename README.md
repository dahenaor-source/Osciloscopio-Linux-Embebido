# Osciloscopio-Linux-Embebido

Proyecto 2 de *Programación de sistemas Linux embebidos* (docente Juan Bernardo Gómez, periodo 2026-02).

Sistema híbrido SBC + microcontrolador que captura **dos señales analógicas** en ventanas de adquisición (como un osciloscopio) y las muestra en un dashboard web.

```
Señales ─► ESP32-C6 (ADC + SPI esclavo) ══ SPI ══► BeaglePlay (SPI maestro + servidor HTTP) ─► Dashboard
```

| Parte | Responsable | Carpeta |
|---|---|---|
| Firmware del microcontrolador (ESP32-C6) | David Henao Rojas | [`firmware/`](firmware) |
| Dashboard web (HTML) | David Henao Rojas (primera versión) | [`dashboard/`](dashboard) |
| Software del SBC (BeaglePlay, SPI maestro, backend, HTTP) | Juan Camilo Giraldo, Daniela Barragán | por definir |

## Documentación

- [**Guía para el equipo del SBC (BeaglePlay)**](docs/guia-sbc.md): qué falta construir, contexto de la BeaglePlay y orden de pruebas. **Juan Camilo y Daniela, empezar aquí.**
- [Dashboard](dashboard/README.md): página HTML con Chart.js (modo simulado) y contrato de la API.
- [**Guía rápida para el equipo**](docs/guia-equipo.md): flashear el binario ya compilado y probar el SPI. **Empezar aquí.**
- [Pinout y conexiones](docs/hardware-pinout.md): mikroBUS de la BeaglePlay ↔ ESP32-C6. **Punto de partida para el equipo del SBC.**
- [Protocolo SPI](docs/protocolo-spi.md): comandos, formato de trama y secuencia de uso.
- [Firmware del microcontrolador](docs/firmware.md): instalación, compilación, tasas teóricas de SPI, pruebas y matriz de verificación.

## Requisitos del enunciado cubiertos por el firmware

| Requisito | Cómo se cumple |
|---|---|
| Comunicación SPI SBC ↔ uC | ESP32-C6 como esclavo SPI2 con DMA, modo 0 |
| Ventanas de ≥ 64 muestras a frecuencia constante | 64 a 1020 muestras por canal, ADC continuo temporizado por hardware |
| Mínimo 3 presets intercambiables por SPI | 100, 1.000, 10.000 y 40.000 Sa/s por canal (comando `SET_RATE`) |
| Bonificación ≥ 40 kSa/s | Preset de 40.000 Sa/s por canal (80 kSa/s en total, límite del ADC: 83,3 kSa/s) |

## Inicio rápido (firmware)

Sin compilar: flashear el binario listo (ver [guía del equipo](docs/guia-equipo.md)):

```bash
pip install esptool
esptool.py --chip esp32c6 -p /dev/ttyACM0 -b 460800 write_flash 0x0 release/osciloscopio_c6_v1.bin
```

Compilando desde el código fuente:
```bash
. ~/esp/esp-idf/export.sh
cd firmware
idf.py set-target esp32c6
idf.py build flash monitor
make -C tests/host test      # pruebas unitarias del protocolo en el PC
```

Detalles en [docs/firmware.md](docs/firmware.md).

## Estado

- [x] Firmware compila (ESP-IDF 5.4.1), se flashea en la ESP32-C6 y arranca (SPI esclavo inicializado). El protocolo pasa sus pruebas unitarias en el PC.
- [ ] Probado con un maestro SPI y con señales reales en el ADC.
- [x] Dashboard HTML con datos simulados (sin backend real).
- [ ] Backend en la BeaglePlay (SPI + API) y servidor HTTP.
- [ ] Integración con la BeaglePlay.
