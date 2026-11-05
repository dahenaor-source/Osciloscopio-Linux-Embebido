# 13. Pendientes y riesgos

Estado a 2026-11-05. Ordenados por prioridad para la entrega.

## 1. Problemas abiertos

| # | Problema | Impacto | Próximo paso | Responsable sugerido |
|---|---|---|---|---|
| A-1 | 🟢 **Resuelto funcionalmente**: Nginx sirve el dashboard y proxya `/api/scope/frame` con HTTP 200 y trama real (P-18) | R-02 verificado; falta hacer reproducible el despliegue | Documentar cómo se instaló nginx y cómo se copió/habilitó `beagleplay/nginx.conf` | Equipo SBC |
| A-2 | 🟡 **El fallo de 40 kSa/s de P-10 no se reprodujo en P-20**: 10/10 capturas sin señal y 5/5 con seno de 1 kHz, `flags=0`, `retries=0` | Evidencia funcional corta; no certifica operación prolongada | Prueba de estrés prolongada aplazada y fuera del alcance de esta etapa por tiempo disponible | Firmware (David) y SBC |
| A-3 | 🟡 **Las pérdidas históricas (~50 %, P-05) no se observaron en P-18/P-20**; 0 reintentos en capturas probadas | El benchmark de 200 transferencias midió caudal, pero no validó cada respuesta SPI ni la operación sostenida | Validación de integridad prolongada aplazada y fuera del alcance de esta etapa por tiempo disponible | Firmware (David) |
| A-4 | 🟡 **Medición preliminar de calibración v2** (4 puntos en CH A, P-21; error absoluto 0,004–0,035 V) | Aún no caracteriza la exactitud: faltan puntos, repeticiones y la incertidumbre del multímetro | Completar la tabla de [08](08-calibracion-adc.md) en CH A y CH B; repetir cada nivel y registrar el instrumento | David |
| A-5 | 🟡 **Frecuencia estimada en los cuatro presets con generador (P-20)**: 100,0 / 1.000,09 / 10.004,06 / 40.307,74 Sa/s | La incertidumbre/calibración del generador no está documentada; estimación a 40 kSa/s difiere +0,77 % del nominal | Confirmar tolerancia del generador o contrastar con analizador lógico/osciloscopio calibrado | Todos |
| A-6 | 🟡 **Caudal medido de 0,10 a 0,54 MB/s entre 1 y 20 MHz (P-20)** | 20 MHz es solo la máxima velocidad probada; no se ha determinado máximo estable ni velocidad real con analizador | Búsqueda de máximo estable y prueba prolongada aplazadas, fuera del alcance de esta etapa por tiempo disponible | Equipo SBC |
| A-7 | **Cómo se habilitó `/dev/spidev0.0` no está documentado** (según el equipo ya venía en la imagen Debian 13.7) | Si otra imagen no lo trae, el resto no puede reproducirlo | Comprobar `ls /dev/spidev*` en una imagen limpia y, si falta, documentar el overlay | Equipo SBC |
| A-8 | Origen del ruido en CH A (¿tráfico SPI o falta de filtrado?) | Calidad de la medida | Repetir sin condensador y con/sin espera sin tráfico SPI para aislar la causa | David y SBC |
| A-9 | 🟡 **Arranque `systemd` probado tras reinicio (P-19); concurrencia API básica pasa en P-20** | La prueba fue corta; falta stress con múltiples clientes | Si se requiere más garantía, repetir `GET /api/scope/frame` y `POST /api/scope/config` con varios clientes simultáneos y verificar respuestas/datos | Equipo SBC |
| A-10 | Numeración mikroBUS sin contrastar con el esquemático de la BeaglePlay | Posible error de cableado | Comparar con la serigrafía y el esquemático | Equipo SBC |
| A-11 | Cobertura de ventana: el dashboard usa la ventana por defecto (256) | A 100 Sa/s el ciclo dura ≥ 2,56 s (≈ 0,38 Hz) | Si se desea refresco más rápido a 100 Sa/s, usar `SET_WINDOW` con ≤ 100 muestras | Decisión del equipo |

## 2. Entregables pendientes

| Entregable | Estado |
|---|---|
| Matriz de verificación con resultados detallados | 🟡 [10](10-matriz-de-verificacion.md): completar las filas ⏳ y 🟡 |
| Video del funcionamiento | ⏳ |
| Declaración de participación y aportes | ⏳ [14](14-participacion-y-uso-de-ia.md) (plantilla) |
| Declaración de uso de IA, con los chats | ⏳ [14](14-participacion-y-uso-de-ia.md) (plantilla) |
| Pruebas unitarias del lado SBC | ⏳ Hay scripts de diagnóstico; no pruebas automatizadas del backend |

## 3. Riesgos

| Riesgo | Probabilidad | Efecto | Mitigación |
|---|---|---|---|
| El fallo histórico a 40 kSa/s pudiera reaparecer bajo señal/carga sostenida | Media | Capturas incompletas o pérdida de la bonificación | Ejecutar más capturas con señal conocida, revisar overrun y conservar logs; P-20 obtuvo 10/10 tramas sin overrun |
| Un comando corrupto pasa el checksum de 8 bits y se ejecuta | Baja (≈ 1/256 por transacción corrupta) | Cambio inesperado de preset o ventana | El backend verifica `rate_idx` y `seq`; si molesta, ampliar el checksum en un protocolo v3 |
| Las pérdidas de transacciones crecen con el tráfico a alta velocidad | Media | Capturas más lentas o fallidas | Resolver A-3 antes de medir A-6 |
| Backend y firmware con versiones distintas del protocolo (v1 con v2) | Media | Valores incorrectos (código crudo leído como mV) | Verificar `fw_version` en la cabecera y mantener una sola versión en el equipo |
| El ADC sin calibración en eFuse | Baja a media | Lecturas con error | El flag bit4 lo avisa; usar tabla de interpolación en el SBC |
| Tolerancia no documentada del generador y falta de analizador lógico | Media | Incertidumbre en la estimación de fs y reloj SPI real | Registrar modelo/especificación del generador y contrastar el reloj SPI con analizador lógico si se requiere más exactitud |
| Entrada analógica sin protección | Baja | Daño del ADC | Resistencia en serie de 1 kΩ, nunca más de 3,3 V |

## 4. Próximos pasos sugeridos (en orden)

1. Completar la caracterización del ADC calibrado (A-4).
2. Documentar la tolerancia del generador y decidir si la precisión de fs y SPI requiere contraste instrumental (A-5/A-6).
3. Documentar la instalación de nginx (A-1), completar [14](14-participacion-y-uso-de-ia.md) y grabar el video.

Las pruebas prolongadas de SPI y de 40 kSa/s no se ejecutarán en esta etapa por tiempo disponible. Los resultados cortos existentes se conservan como evidencia funcional, no como certificación de estabilidad sostenida.
