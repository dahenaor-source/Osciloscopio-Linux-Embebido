# 16. Plan de validación de integración — Osc-V3

**Corte documental:** 5 de noviembre de 2026. **Rama base revisada:** `Osc-V3`, commit `f18c41b`. Este plan es una lista de **pruebas por ejecutar**, no un registro de resultados obtenidos.

## 1. Alcance y estado de la evidencia

El código incluye firmware ESP32-C6 con ADC continuo y SPI esclavo, backend FastAPI en la BeaglePlay, interfaz web, configuración nginx y un instalador de servicio systemd. La existencia de estos componentes no garantiza una adquisición correcta en la configuración actual.

Historial disponible: firmware v2 flasheado y arrancado en placa el 2026-11-04, con calibración eFuse inicializada; nginx sirvió el dashboard con HTTP 200 el 2026-11-05; el acceso a `/api/` produjo 502 cuando el backend estaba apagado; se documentaron capturas antiguas con potenciómetro y transacciones SPI inválidas. **No consta una prueba completa y repetible de la rama Osc-V3 en hardware.**

## 2. Lista de aceptación

| ID | Verificación | Evidencia exigida | Estado |
|---|---|---|---|
| V3-01 | ESP32-C6 arranca con binario compilado desde Osc-V3 | `git rev-parse --short HEAD`, salida de `idf.py build`, hash de flasheo y `App version` del monitor | Pendiente |
| V3-02 | BeaglePlay inicia backend tras reinicio | `systemctl is-enabled/is-active`, `journalctl` y `curl` local | Pendiente |
| V3-03 | nginx → FastAPI | `curl -i http://127.0.0.1/api/scope/frame` con backend activo; distinguir HTTP 200, 500 y 502 | Pendiente |
| V3-04 | SPI estable a 2 MHz | 100 capturas sucesivas: éxitos, errores, reintentos, latencias y `seq` | Pendiente |
| V3-05 | ADC físico (no simulado) | Entrada GND y nivel DC conocido en ambos canales, medición simultánea con multímetro | Pendiente |
| V3-06 | Frecuencias 100, 1k, 10k Sa/s | Generador conocido: muestras por período, `fs_hz` de la respuesta y ventana | Pendiente |
| V3-07 | 40 kSa/s | Evidencia de captura íntegra, ausencia de overrun, frecuencia efectiva y repetición | Pendiente; falló históricamente |
| V3-08 | Selector y refresco del dashboard | Video o captura con reloj: fuente API real, cambios de preset y actualización sin ciclos duplicados | Pendiente en hardware |
| V3-09 | Integridad de datos | Separación modo simulado / modo de prueba SPI / captura ADC real; comportamiento al desconectar ESP32 | Pendiente |
| V3-10 | Entrega | Video, matriz, identificación de aportes y declaración de uso de IA revisada por cada integrante | Pendiente |

## 3. Comandos de referencia

### En Ubuntu, desde la raíz del repositorio

```bash
git branch --show-current
git rev-parse --short HEAD
git status -sb
cd firmware
source ~/esp/esp-idf/export.sh
idf.py build
ls -l /dev/ttyACM* /dev/ttyUSB* 2>/dev/null
# Usar el puerto que realmente aparezca, no asumir ttyACM0.
# idf.py -p /dev/ttyACM0 flash monitor
```

### En BeaglePlay

```bash
ls -l /dev/spidev*
systemctl is-enabled osciloscopio-backend
systemctl is-active osciloscopio-backend
sudo journalctl -u osciloscopio-backend -n 80 --no-pager
curl -i http://127.0.0.1/
curl -i http://127.0.0.1/api/scope/frame
```

**Interpretación:** HTTP 200 para `/` valida el servidor estático. HTTP 502 para `/api/` indica fallo de comunicación con el upstream; HTTP 500 puede corresponder a error de adquisición. Solo una respuesta de trama real válida, corroborada en el instrumento, constituye evidencia de captura.

## 4. Registro reproducible por sesión

Registrar fecha, rama y commit de **ambas placas**, binario realmente flasheado, esquema de conexiones, frecuencia SPI, preset, longitud de ventana, señal aplicada y medida independiente; conservar logs seriales, del backend y respuestas HTTP. No mezclar evidencias de firmware v1, v2 y cambios recientes sin etiquetar su versión.

## 5. Problemas no resueltos

- Respuestas SPI inválidas y necesidad de rearmar el esclavo: causa raíz no demostrada.
- Exactitud del ADC tras calibración eFuse: no hay tabla experimental nueva.
- Preset 40 kSa/s: no puede declararse bonificación cumplida.
- Servicio systemd: el instalador existe, pero falta demostrar que sobrevive al reinicio de la BeaglePlay y adquiere datos.
- El modo simulado del navegador y el modo de prueba SPI **no** son mediciones físicas.

Documentos relacionados: [09 Pruebas](09-pruebas-y-resultados.md), [10 Matriz](10-matriz-de-verificacion.md), [13 Pendientes](13-pendientes-y-riesgos.md) y [15 Flasheo V2](15-verificacion-flasheo-v2.md).
