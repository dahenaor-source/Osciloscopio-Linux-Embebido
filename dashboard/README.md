# Dashboard

Página estática (HTML + JS + Chart.js local, sin CDN: la red del SBC puede no tener internet). En la BeaglePlay, nginx sirve el dashboard por el puerto 80 y hace proxy de `/api/` a FastAPI. La integración se verificó mediante HTTP 200 en P-18.

## Probar la interfaz (modo simulado; no es medición física)

```bash
cd dashboard && python3 -m http.server 8080
```

Abre `http://localhost:8080` → **Start**. Muestra 2 canales simulados, estadísticas (mín, máx, pp, promedio, RMS) y permite cambiar preset y refresco (0,25 / 0,5 / 1 Hz; por defecto 0,25 Hz, dentro del requisito 0,25–1 Hz).

## Archivos

| Archivo | Función |
|---|---|
| `index.html` | Estructura y controles |
| `scope.js` | Fuente simulada, cliente de la API, gráfica, estadísticas |
| `style.css` | Estilo |
| `vendor/chart.umd.js` | Chart.js 4.6.1 (local) |

## Contrato de API (lo que debe implementar el backend del SBC)

**`GET /api/scope/frame`** → JSON con la última ventana capturada:

```json
{ "seq": 12, "rate_idx": 1, "fs_hz": 1000, "window": 256, "flags": 0,
  "retries": 0, "a": [1650, 1655, ...], "b": [820, 818, ...] }
```

- `a`, `b`: milivoltios (protocolo v2, ya calibrados en el ESP32 con la eFuse del ADC; ver `docs/Documentación_General/04-protocolo-spi.md`). El dashboard convierte a voltios: `V = mV / 1000`.
- `flags`: los del encabezado SPI (bit0 overrun, bit1 comando rechazado, bit3 modo test, bit4 ADC sin calibrar). El backend enmascara el bit2.
- `retries` (opcional): transacciones SPI repetidas en esa captura; el dashboard lo muestra en la barra de estado.

**`POST /api/scope/config`** con `{"rate_idx": 0..3}` → el backend envía `SET_RATE` por SPI.

Para usarlo: en el dashboard elige la fuente **API real**. Con nginx/apache se puede hacer proxy de `/api/` a un proceso del SBC (Python/C) que hable SPI.

## Pendiente

- Conservar evidencia de la prueba P-18 del proxy `/api/`; repetir la validación después de cambios del backend (nginx ya sirve esta carpeta en el puerto 80; el backend FastAPI también la sirve en el 8000).
- Escala fija 0–3,3 V; añadir auto-escala y selector de ventana si se requiere.
- Verificar en hardware el refresco real con captura a 40 kSa/s.

Documentación completa: [`docs/Documentación_General/07-dashboard.md`](../docs/Documentación_General/07-dashboard.md).
