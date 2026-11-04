# 15. Verificación del flasheo ESP32-C6 V2

## Fecha
4 de noviembre de 2026.

## Objetivo
Verificar la compilación, programación e inicialización del
firmware V2 del osciloscopio en la ESP32-C6.

## Hardware y software

- Microcontrolador: ESP32-C6, revisión v0.2.
- Entorno: Ubuntu.
- Framework: ESP-IDF 5.4.1.
- Versión del firmware: b3b4318.
- Puerto de programación: /dev/ttyACM0.

## Procedimiento

Se ejecutaron los comandos:

    source ~/esp/esp-idf/export.sh
    cd ~/Osciloscopio-Linux-Embebido/firmware
    idf.py build
    idf.py -p /dev/ttyACM0 flash monitor

## Resultados

La compilación terminó correctamente.

La escritura de la memoria flash se completó
y los bloques fueron verificados mediante hash.

El firmware arrancó correctamente.

El monitor serie mostró:

    App version: b3b4318
    ESP-IDF: v5.4.1

    capture: canal 0: calibracion curve-fitting (eFuse)
    capture: canal 1: calibracion curve-fitting (eFuse)

    scope_link: SPI esclavo listo
    (SCLK=6 MOSI=7 MISO=2 CS=10 HS=3)

## Conclusiones

Se verificó satisfactoriamente:

1. Compilación del firmware V2.
2. Programación de la ESP32-C6.
3. Integridad de la imagen grabada.
4. Arranque de la aplicación.
5. Inicialización de calibración eFuse en ambos canales.
6. Inicialización del SPI esclavo.

## Pruebas pendientes

- Exactitud del ADC calibrado frente a un multímetro.
- Comunicación SPI completa usando firmware V2.
- Captura y transmisión de muestras a 40 kSa/s.
- Verificación experimental de frecuencias de muestreo.

La inicialización exitosa de eFuse no demuestra por sí
sola la exactitud de las mediciones.

## Estado

APROBADO: compilación, flasheo y arranque.

PENDIENTE: validación funcional y metrológica completa.
