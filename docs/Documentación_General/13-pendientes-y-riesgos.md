# 13. Pendientes y riesgos

Estado a 2026-11-05. Ordenados por prioridad para la entrega.

## 1. Problemas abiertos

| # | Problema | Impacto | Próximo paso | Responsable sugerido |
|---|---|---|---|---|
| A-1 | 🟢 **Resuelto funcionalmente**: Nginx sirve el dashboard y proxya `/api/scope/frame` con HTTP 200 y trama real (P-18) | R-02 verificado; falta hacer reproducible el despliegue | Documentar cómo se instaló nginx y cómo se copió/habilitó `beagleplay/nginx.conf` | Equipo SBC |
| A-2 | **Preset de 40 kSa/s con error constante** | Sin él no hay bonificación (R-08) | Registrar el mensaje exacto: log del backend (`[ERROR REST] ...`), flag `OVERRUN` del dashboard y monitor serie del ESP32-C6 (`capture: captura fallida`) | Firmware (David) y SBC |
| A-3 | **El esclavo SPI pierde ≈ 50 % de las transacciones** | Más latencia y riesgo de comandos corruptos; sin causa raíz | Añadir contador de transacciones y log de `trans_len` y primeros bytes en el firmware; probar la línea INT como sincronización; comparar velocidades y cables más cortos | Firmware (David) |
| A-4 | **Calibración v2: falta medir la exactitud** (el arranque en hardware ya está verificado: calibra por eFuse en ambos canales) | No se sabe cuánto mejora el error | Repetir la tabla de [08](08-calibracion-adc.md) con multímetro | David |
| A-5 | **Frecuencia de muestreo real sin medir** | R-04 y FW-08 sin evidencia experimental | Señal periódica conocida (generador, o PWM + filtro RC generado por el ESP32-C6) y contar muestras por periodo | Todos |
| A-6 | **Tasa máxima de SPI y caudal sin medir** | R-07 sin verificación experimental | Medir `xfer2` a 1 a 20 MHz y registrar dónde aparecen errores | Equipo SBC |
| A-7 | **Cómo se habilitó `/dev/spidev0.0` no está documentado** (según el equipo ya venía en la imagen Debian 13.7) | Si otra imagen no lo trae, el resto no puede reproducirlo | Comprobar `ls /dev/spidev*` en una imagen limpia y, si falta, documentar el overlay | Equipo SBC |
| A-8 | Origen del ruido en CH A (¿tráfico SPI o falta de filtrado?) | Calidad de la medida | Repetir sin condensador y con/sin espera sin tráfico SPI para aislar la causa | David y SBC |
| A-9 | **Servicio `systemd` activo y habilitado en la placa (P-18); falta probar después de reinicio y concurrencia** | Si falla el arranque automático, el dashboard abre por nginx pero no muestra datos | Reiniciar la BeaglePlay y comprobar API/dashboard sin intervención; después probar `GET` y `POST` simultáneos (comandos en [06](06-backend-beagleplay.md), sección 3) | Equipo SBC |
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
| El problema de 40 kSa/s es un límite del ADC o del hardware | Media | Se pierde la bonificación | Reducir a un preset estable (p. ej. 30 kSa/s) y documentar el máximo certificado |
| Un comando corrupto pasa el checksum de 8 bits y se ejecuta | Baja (≈ 1/256 por transacción corrupta) | Cambio inesperado de preset o ventana | El backend verifica `rate_idx` y `seq`; si molesta, ampliar el checksum en un protocolo v3 |
| Las pérdidas de transacciones crecen con el tráfico a alta velocidad | Media | Capturas más lentas o fallidas | Resolver A-3 antes de medir A-6 |
| Backend y firmware con versiones distintas del protocolo (v1 con v2) | Media | Valores incorrectos (código crudo leído como mV) | Verificar `fw_version` en la cabecera y mantener una sola versión en el equipo |
| El ADC sin calibración en eFuse | Baja a media | Lecturas con error | El flag bit4 lo avisa; usar tabla de interpolación en el SBC |
| Falta de instrumentos (generador, analizador lógico) | Alta | FW-08 y reloj SPI sin medir | Usar el ESP32-C6 como generador de PWM, o pedir prestado el equipo en el laboratorio |
| Entrada analógica sin protección | Baja | Daño del ADC | Resistencia en serie de 1 kΩ, nunca más de 3,3 V |

## 4. Próximos pasos sugeridos (en orden)

1. Recuperar el enlace SPI (P-14 en [09](09-pruebas-y-resultados.md)), registrar el error de 40 kSa/s (A-2) y medir la exactitud calibrada (A-4).
2. Probar el arranque automático del backend en la placa (A-9) y confirmar con él el proxy de nginx (A-1).
3. Medir SPI (A-6) y contar transacciones perdidas (A-3).
4. Generar una señal periódica y medir fs (A-5).
5. Completar [10](10-matriz-de-verificacion.md), [14](14-participacion-y-uso-de-ia.md) y grabar el video.
