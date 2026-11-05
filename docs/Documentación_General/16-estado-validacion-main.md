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
| P-20 | 10 capturas consecutivas a 40 kSa/s, sin flags ni reintentos | Frecuencia efectiva no medida con señal conocida |

## 2. Validaciones pendientes

| ID | Verificación requerida | Estado |
|---|---|---|
| V-01 | Exactitud del ADC con multímetro | Pendiente |
| V-02 | Frecuencia real de muestreo con señal conocida | Pendiente |
| V-03 | Caudal SPI máximo y estabilidad prolongada | Pendiente |
| V-04 | Ensayo prolongado de 40 kSa/s con señal conocida | Pendiente |
| V-05 | Prueba extendida de concurrencia con varios clientes | Pendiente |
| V-06 | Video del funcionamiento del sistema | Pendiente |
| V-07 | Declaración de aportes y uso de IA, revisada por el equipo | Pendiente |

## 3. Criterio documental

Las pruebas históricas se conservan con su versión y fecha.
Una captura válida a 40 kSa/s no demuestra, por sí sola,
que la frecuencia efectiva del ADC sea exactamente 40 kSa/s.

Las observaciones que contradicen resultados antiguos se
documentan como evolución del sistema, no se eliminan.

## 4. Documentos de referencia

- [09. Pruebas y resultados](09-pruebas-y-resultados.md)
- [10. Matriz de verificación](10-matriz-de-verificacion.md)
- [13. Pendientes y riesgos](13-pendientes-y-riesgos.md)
