"use strict";

// ---- Contrato con el backend (ver dashboard/README.md) -------------------
const API_FRAME  = "/api/scope/frame";   // GET  -> {seq, rate_idx, fs_hz, window, flags, a:[raw], b:[raw]}
const API_CONFIG = "/api/scope/config";  // POST <- {rate_idx}

// Mismos presets que el firmware (docs/protocolo-spi.md)
const PRESETS = [
  { idx: 0, fs: 100,   label: "100 Sa/s" },
  { idx: 1, fs: 1000,  label: "1.000 Sa/s" },
  { idx: 2, fs: 10000, label: "10.000 Sa/s" },
  { idx: 3, fs: 40000, label: "40.000 Sa/s" },
];
const ADC_MAX = 4095;   // 12 bits
const V_FULL  = 3.1;    // V ≈ raw / 4095 * 3,1 (atenuación 12 dB)
const WINDOW  = 256;    // muestras por ventana en modo simulado

const $ = (id) => document.getElementById(id);
const toVolts = (raw) => (raw / ADC_MAX) * V_FULL;

// ---- Fuente simulada: 2 senoidales con ruido, en la ventana pedida ------
let simPhase = 0;
function simFrame(rateIdx) {
  const p = PRESETS[rateIdx];
  // Frecuencias elegidas para que siempre se vean ~4 y ~7 ciclos por ventana
  const fA = (p.fs / WINDOW) * 4, fB = (p.fs / WINDOW) * 7;
  const a = [], b = [];
  for (let n = 0; n < WINDOW; n++) {
    const t = n / p.fs;
    const va = 1.55 + 1.2 * Math.sin(2 * Math.PI * fA * t + simPhase) + (Math.random() - 0.5) * 0.03;
    const vb = 1.0 + 0.8 * (((fB * t) % 1) < 0.5 ? 1 : -1) * 0.5 + (Math.random() - 0.5) * 0.03;
    a.push(Math.round(va / V_FULL * ADC_MAX));
    b.push(Math.round(vb / V_FULL * ADC_MAX));
  }
  simPhase += 0.7;
  return { seq: (simFrame.seq = ((simFrame.seq || 0) + 1) & 0xffff), rate_idx: rateIdx,
           fs_hz: p.fs, window: WINDOW, flags: 0, a, b };
}

// ---- Fuente real: HTTP hacia el servidor del SBC -------------------------
async function apiFrame() {
  const r = await fetch(API_FRAME, { cache: "no-store" });
  if (!r.ok) throw new Error("HTTP " + r.status);
  return r.json();
}
async function apiSetRate(idx) {
  await fetch(API_CONFIG, {
    method: "POST", headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ rate_idx: idx }),
  });
}

// ---- Gráfica -------------------------------------------------------------
const chart = new Chart($("chart"), {
  type: "line",
  data: { labels: [], datasets: [
    { label: "CH A", borderColor: "#4cc9f0", data: [], pointRadius: 0, borderWidth: 1.5 },
    { label: "CH B", borderColor: "#f9c74f", data: [], pointRadius: 0, borderWidth: 1.5 },
  ]},
  options: {
    animation: false, responsive: true, maintainAspectRatio: false,
    scales: {
      y: { min: 0, max: 3.3, title: { display: true, text: "Voltaje (V)", color: "#8b98a8" },
           ticks: { color: "#8b98a8" }, grid: { color: "#2c3746" } },
      x: { title: { display: true, text: "Tiempo (ms)", color: "#8b98a8" },
           ticks: { color: "#8b98a8", maxTicksLimit: 10 }, grid: { color: "#2c3746" } },
    },
    plugins: { legend: { labels: { color: "#e6edf3" } } },
  },
});

function stats(v) {
  const n = v.length;
  const min = Math.min(...v), max = Math.max(...v);
  const avg = v.reduce((s, x) => s + x, 0) / n;
  const rms = Math.sqrt(v.reduce((s, x) => s + x * x, 0) / n);
  return [min, max, max - min, avg, rms];
}
function fillRow(id, v) {
  const cells = $(id).querySelectorAll("td");
  stats(v).forEach((x, i) => { cells[i + 1].textContent = x.toFixed(3); });
}

function render(f) {
  const va = f.a.map(toVolts), vb = f.b.map(toVolts);
  // Eje X en ms según la frecuencia de muestreo reportada por el ESP32
  chart.data.labels = va.map((_, n) => (n / f.fs_hz * 1000).toFixed(n && f.fs_hz >= 10000 ? 2 : 1));
  chart.data.datasets[0].data = va;
  chart.data.datasets[1].data = vb;
  chart.update();
  fillRow("row-a", va);
  fillRow("row-b", vb);
  const warn = [];
  if (f.flags & 1) warn.push("OVERRUN");
  if (f.flags & 2) warn.push("comando rechazado");
  $("info").textContent =
    `seq ${f.seq} · ${f.fs_hz} Sa/s · ${f.window} muestras · ventana ${(f.window / f.fs_hz * 1000).toFixed(1)} ms` +
    (warn.length ? " · ⚠ " + warn.join(", ") : "") + (f.flags & 8 ? " · modo test" : "") +
    (f.retries ? ` · reintentos SPI: ${f.retries}` : "");
}

// ---- Control -------------------------------------------------------------
let timer = null;

function setStatus(text, cls) {
  const s = $("status");
  s.textContent = text;
  s.className = "badge " + (cls || "");
}

async function tick() {
  const idx = Number($("sel-rate").value);
  try {
    const f = $("sel-source").value === "sim" ? simFrame(idx) : await apiFrame();
    // El uC conserva su último preset: si no coincide con el seleccionado, se lo reenvía.
    if ($("sel-source").value === "api" && f.rate_idx !== idx) {
      await apiSetRate(idx);
      setStatus("ajustando frecuencia...", "");
      return;
    }
    render(f);
    setStatus("capturando", "ok");
  } catch (e) {
    setStatus("error: " + e.message, "err");
  }
}

function start() {
  stop();
  tick();
  timer = setInterval(tick, Number($("sel-refresh").value));
  $("btn-toggle").textContent = "■ Stop";
  $("btn-toggle").classList.add("running");
}
function stop() {
  if (timer) clearInterval(timer);
  timer = null;
  $("btn-toggle").textContent = "▶ Start";
  $("btn-toggle").classList.remove("running");
  setStatus("detenido");
}

PRESETS.forEach((p) => $("sel-rate").add(new Option(p.label, p.idx)));
$("sel-rate").value = 1;

$("btn-toggle").onclick = () => (timer ? stop() : start());
$("sel-refresh").onchange = () => { if (timer) start(); };
$("sel-source").onchange = () => { if (timer) start(); };
$("sel-rate").onchange = async () => {
  if ($("sel-source").value === "api") {
    try { await apiSetRate(Number($("sel-rate").value)); } catch (e) { setStatus("error: " + e.message, "err"); }
  }
  if (timer) start();
};
