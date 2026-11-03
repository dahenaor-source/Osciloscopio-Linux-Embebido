# Documentación general: Osciloscopio SBC + microcontrolador

Proyecto 2 de *Programación de sistemas Linux embebidos* (docente Juan Bernardo Gómez, periodo 2026-02). Esta carpeta reúne **toda la documentación del proyecto hasta el 2026-11-03**: descripción, requisitos, hardware, protocolo, firmware, backend, dashboard, calibración, resultados de pruebas, matriz de verificación y pendientes.

> **Estado en una línea:** el enlace SPI BeaglePlay ↔ ESP32-C6 funciona a 2 MHz y el dashboard muestra datos reales del ADC (1 kSa/s probado con un potenciómetro). Falta cerrar el preset de 40 kSa/s, la causa de las transacciones SPI perdidas, el servidor apache/nginx y las mediciones con instrumentos. Detalle en [13-pendientes-y-riesgos.md](13-pendientes-y-riesgos.md).

## Índice

| # | Documento | Contenido |
|---|---|---|
| 01 | [Descripción general](01-descripcion-general.md) | Objetivo, arquitectura, diagramas de bloques y de flujo, decisiones de diseño |
| 02 | [Requerimientos y restricciones](02-requerimientos-y-restricciones.md) | Requisitos del enunciado y su estado, restricciones técnicas |
| 03 | [Hardware y conexiones](03-hardware-y-conexiones.md) | Pinout mikroBUS ↔ ESP32-C6, entradas analógicas, montaje de prueba |
| 04 | [Protocolo SPI](04-protocolo-spi.md) | Comandos, cabecera, formato de datos (versión 2) |
| 05 | [Firmware del ESP32-C6](05-firmware-esp32c6.md) | Arquitectura, entorno, compilación, tasas teóricas de SPI |
| 06 | [Backend en la BeaglePlay](06-backend-beagleplay.md) | FastAPI, driver SPI, reintentos, API |
| 07 | [Dashboard](07-dashboard.md) | Interfaz web, contrato de la API, capturas |
| 08 | [Calibración del ADC](08-calibracion-adc.md) | Problema medido, solución en firmware, cómo verificarla |
| 09 | [Pruebas y resultados](09-pruebas-y-resultados.md) | Bitácora cronológica con evidencia y hallazgos |
| 10 | [Matriz de verificación](10-matriz-de-verificacion.md) | Requisito → prueba → resultado |
| 11 | [Guía: flashear y probar el ESP32-C6](11-guia-flasheo-esp32c6.md) | Para cualquier miembro del equipo, sin instalar ESP-IDF |
| 12 | [Guía para el equipo del SBC](12-guia-equipo-sbc.md) | Contexto de la BeaglePlay y orden de pruebas |
| 13 | [Pendientes y riesgos](13-pendientes-y-riesgos.md) | Qué falta, problemas abiertos, próximos pasos |
| 14 | [Participación y uso de IA](14-participacion-y-uso-de-ia.md) | Roles, aportes y declaración de uso de IA (por completar por el equipo) |

## Por dónde empezar

- **Quiero entender el proyecto:** 01 → 02 → 03.
- **Quiero flashear la placa y probar:** 11, luego 03 para el cableado.
- **Soy del equipo del SBC:** 12, 06 y 04.
- **Quiero ver qué está probado y qué no:** 09 y 10.

## Mapa del repositorio

```
Osciloscopio-Linux-Embebido/
├── firmware/            Código del ESP32-C6 (ESP-IDF 5.4.1) y pruebas unitarias del protocolo (tests/host)
├── beagleplay/backend/  Backend FastAPI y driver SPI para la BeaglePlay, y scripts de diagnóstico
├── dashboard/           Página web (HTML + JS + Chart.js local)
├── tools/               Maestro SPI de referencia en Python (spi_master_test.py)
├── release/             Binario del firmware listo para flashear (osciloscopio_c6_v2.bin)
└── docs/Documentación_General/   Esta documentación (y su carpeta img/)
```

## Convenciones

- Frecuencias de muestreo **por canal**: 100, 1.000, 10.000 y 40.000 Sa/s (presets 0 a 3).
- Los voltajes se entregan en **milivoltios calibrados** desde el protocolo v2 (antes, código crudo de 12 bits).
- ✅ verificado · 🟡 parcial · ⏳ pendiente · ❌ falla o no cumple · ⚠️ supuesto sin confirmar.
- Las afirmaciones de esta documentación se apoyan en pruebas reales cuando se indica evidencia en [09](09-pruebas-y-resultados.md); lo demás se marca como pendiente o supuesto.
