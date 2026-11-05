# 16. Estado de validación de la rama main

**Fecha de corte:** 5 de noviembre de 2026.

Este documento complementa la matriz de verificación y
la bitácora de pruebas. Se basa en las evidencias registradas
hasta P-20 y no presenta pruebas pendientes como cumplidas.

## 1. Resultados comprobados

| Evidencia | Resultado | Alcance |
|---|---|---|
| P-13 | Firmware V2 arrancó con calibración eFuse en ambos canales | No certifica exactitud del ADC |
| P-18 | nginx y API respondieron HTTP 200 con trama de 256 muestras por canal | Integración observada en una sesión |
| P-19 | Backend arrancó tras reiniciar la BeaglePlay | Confirmación registrada por el equipo |
| P-20 | Pruebas concurrentes de GET y POST completadas correctamente | No equivale a prueba prolongada de carga |
| P-20 | Frecuencia estimada en los cuatro presets: 100,0; 1.000,09; 10.004,06 y 40.307,74 Sa/s | Estimación basada en las frecuencias nominales del generador; su tolerancia no está documentada |
| P-20 | 15 capturas a 40 kSa/s válidas (10 sin señal y 5 con seno conocido de 1 kHz), sin flags ni reintentos | No equivale a una prueba prolongada de estrés |
| P-20 | Caudal SPI medido: 0,10; 0,18; 0,33; 0,47 y 0,54 MB/s a 1, 2, 5, 10 y 20 MHz solicitados | El caudal incluye sobrecarga del SO; no certifica el reloj real ni la máxima velocidad estable |
| P-21 | Comparación ADC v2 en CH A: 4 puntos, error absoluto 0,004–0,035 V | Medición preliminar con potenciómetro; faltan puntos, repeticiones e incertidumbre del multímetro |

## 2. Validaciones pendientes

| ID | Verificación requerida | Estado |
|---|---|---|
| V-01 | Exactitud del ADC con multímetro | Parcial: cuatro puntos iniciales en CH A (P-21); completar niveles, repeticiones y caracterizar ambos canales |
| V-02 | Confirmar frecuencia de muestreo con referencia calibrada | Estimaciones disponibles en P-20; falta documentar tolerancia del generador o contrastar con instrumento calibrado |
| V-03 | Caudal SPI máximo y estabilidad prolongada | Caudal medido entre 1 y 20 MHz solicitados; máximo estable y reloj real pendientes |
| V-04 | Ensayo prolongado de 40 kSa/s con señal conocida | 5 capturas con seno de 1 kHz pasaron; ensayo prolongado pendiente |
| V-05 | Prueba extendida de concurrencia con varios clientes | 10 pares simultáneos GET/POST pasaron; carga prolongada con varios clientes pendiente |
| V-06 | Video del funcionamiento del sistema | Pendiente |
| V-07 | Declaración de aportes y uso de IA, revisada por el equipo | Pendiente |

## 3. Criterio documental

Las pruebas históricas se conservan con su versión y fecha.
Las estimaciones de frecuencia de P-20 usan los valores nominales
del generador y no certifican la exactitud de muestreo. Del mismo
modo, el benchmark SPI reporta caudal efectivo, no el reloj real ni
la máxima velocidad estable.

Las observaciones que contradicen resultados antiguos se
documentan como evolución del sistema, no se eliminan.

## 4. Documentos de referencia

- [09. Pruebas y resultados](09-pruebas-y-resultados.md)
- [10. Matriz de verificación](10-matriz-de-verificacion.md)
- [13. Pendientes y riesgos](13-pendientes-y-riesgos.md)
