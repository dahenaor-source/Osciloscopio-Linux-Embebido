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
| Software del SBC (BeaglePlay, SPI maestro, backend, HTTP) | Juan Camilo Giraldo, Daniela Barragán | [`beagleplay/backend/`](beagleplay/backend) |

## Documentación

**¿Quieres armarlo tú?** Sigue [**Instrucciones de uso**](docs/Instrucciones_De_Uso/Instrucciones_De_Uso.md): guía paso a paso desde cero (materiales, flasheo, montaje y uso).

Toda la documentación técnica está en [**`docs/Documentación_General/`**](docs/Documentaci%C3%B3n_General/README.md) (índice con los 12 documentos):

| Documento | Contenido |
|---|---|
| [01 Descripción general](docs/Documentaci%C3%B3n_General/01-descripcion-general.md) | Objetivo, arquitectura, diagramas de bloques y de flujo |
| [02 Requerimientos y restricciones](docs/Documentaci%C3%B3n_General/02-requerimientos-y-restricciones.md) | Requisitos del enunciado y su estado |
| [03 Hardware y conexiones](docs/Documentaci%C3%B3n_General/03-hardware-y-conexiones.md) | Pinout mikroBUS ↔ ESP32-C6 y diagramas |
| [04 Protocolo SPI](docs/Documentaci%C3%B3n_General/04-protocolo-spi.md) | Comandos y formato de trama (v2) |
| [05 Firmware](docs/Documentaci%C3%B3n_General/05-firmware-esp32c6.md) | Arquitectura, compilación, tasas teóricas de SPI |
| [06 Backend BeaglePlay](docs/Documentaci%C3%B3n_General/06-backend-beagleplay.md) | FastAPI, driver SPI y reintentos |
| [07 Dashboard](docs/Documentaci%C3%B3n_General/07-dashboard.md) | Interfaz web y capturas |
| [08 Calibración del ADC](docs/Documentaci%C3%B3n_General/08-calibracion-adc.md) | Error medido y calibración por eFuse |
| [09 Pruebas y resultados](docs/Documentaci%C3%B3n_General/09-pruebas-y-resultados.md) | Bitácora con evidencia |
| [10 Matriz de verificación](docs/Documentaci%C3%B3n_General/10-matriz-de-verificacion.md) | Requisito → prueba → resultado |
| [13 Pendientes y riesgos](docs/Documentaci%C3%B3n_General/13-pendientes-y-riesgos.md) | Qué falta |
| [14 Participación y uso de IA](docs/Documentaci%C3%B3n_General/14-participacion-y-uso-de-ia.md) | Participación del equipo y uso documentado de IA |

Además: [dashboard/README.md](dashboard/README.md) y [beagleplay/backend/README.md](beagleplay/backend/README.md).

## Requisitos del enunciado cubiertos por el firmware

| Requisito | Cómo se cumple |
|---|---|
| Comunicación SPI SBC ↔ uC | ESP32-C6 como esclavo SPI2 con DMA, modo 0 |
| Ventanas de ≥ 64 muestras a frecuencia constante | 64 a 1020 muestras por canal, ADC continuo temporizado por hardware |
| Mínimo 3 presets intercambiables por SPI | 100, 1.000, 10.000 y 40.000 Sa/s por canal (comando `SET_RATE`) |
| Bonificación ≥ 40 kSa/s | Preset de 40.000 Sa/s por canal implementado. P-20 obtuvo 15/15 capturas válidas, incluidas 5 con seno de 1 kHz; frecuencia estimada ≈ 40.308 Sa/s. Falta prueba de estrés prolongada y considerar la tolerancia del generador. |

## Inicio rápido (firmware)

Sin compilar: flashear el binario listo (ver [guía de flasheo](docs/Instrucciones_De_Uso/Instrucciones_De_Uso.md)):

```bash
pip install esptool
esptool.py --chip esp32c6 -p /dev/ttyACM0 -b 460800 write_flash 0x0 release/osciloscopio_c6_v2.bin
```

Compilando desde el código fuente:
```bash
. ~/esp/esp-idf/export.sh
cd firmware
idf.py set-target esp32c6
idf.py build flash monitor
make -C tests/host test      # pruebas unitarias del protocolo en el PC
```

Detalles en [05-firmware-esp32c6.md](docs/Documentaci%C3%B3n_General/05-firmware-esp32c6.md).

## Estado verificado (2026-11-05)

- [x] Firmware compila (ESP-IDF 5.4.1), se flashea y arranca. Protocolo con pruebas unitarias en el PC.
- [x] Enlace SPI BeaglePlay ↔ ESP32-C6 funcionando a 2 MHz. Históricamente se observaron pérdidas de aproximadamente el 50 % de las transacciones (P-05), pero las capturas recientes P-18, P-20 y P-22 fueron válidas, sin reintentos reportados. La prueba prolongada de integridad queda fuera del alcance de esta etapa.
- [x] Dashboard con datos reales (modo de prueba y ADC con potenciómetro, 100 a 10.000 Sa/s).
- [x] Backend FastAPI en la BeaglePlay (SPI + API) con reintentos.
- [x] Firmware v2: calibración del ADC por eFuse (arranque verificado en ambos canales; P-21 registró una comparación preliminar de cuatro puntos en CH A, aún falta completar la caracterización).
- [x] Preset de 40 kSa/s: 15 capturas API válidas en P-20, incluidas 5 con seno conocido de 1 kHz; frecuencia estimada ≈ 40.308 Sa/s. Prueba prolongada fuera del alcance de esta etapa.
- [x] Servidor nginx activo y sirviendo el dashboard en el puerto 80 (verificado el 2026-11-05); el proxy `/api/` fue verificado con HTTP 200 en P-18.
- [x] Arranque automático del backend con systemd (`beagleplay/instalar_servicio.sh`): verificado tras reiniciar la BeaglePlay (P-19).
- [x] Frecuencia de muestreo estimada con generador en los cuatro presets; caudal SPI medido entre 1 y 20 MHz. La frecuencia nominal del generador no está calibrada y el máximo SPI estable no se ha determinado.
- [ ] Completar la caracterización del ADC v2 (medición preliminar de cuatro puntos registrada en P-21) y documentar tolerancia de la referencia del generador. Pruebas prolongadas de SPI y 40 kSa/s fuera del alcance de esta etapa.
- [ ] Video, declaración de participación y de uso de IA.

Detalle en [13-pendientes-y-riesgos.md](docs/Documentaci%C3%B3n_General/13-pendientes-y-riesgos.md).
