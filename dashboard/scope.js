"use strict";

// ---- Contrato con el backend (ver dashboard/README.md) -------------------
const API_FRAME  = "/api/scope/frame";   // GET  -> {seq, rate_idx, fs_hz, window, flags, a:[mV], b:[mV]}
const API_CONFIG = "/api/scope/config";  // POST <- {rate_idx}

// Mismos presets que el firmware (docs/Documentación_General/04-protocolo-spi.md)
const PRESETS = [
  { idx: 0, fs: 100,   label: "100 Sa/s" },
  { idx: 1, fs: 1000,  label: "1.000 Sa/s" },
  { idx: 2, fs: 10000, label: "10.000 Sa/s" },
  { idx: 3, fs: 40000, label: "40.000 Sa/s" },
];

const WINDOW = 256;    // Muestras por ventana
const $ = (id) => document.getElementById(id);
const toVolts = (mv) => mv / 1000;

// ---- Fuente simulada -----------------------------------------------------
let simPhase = 0;
function simFrame(rateIdx) {
  const p = PRESETS[rateIdx];
  const fA = (p.fs / WINDOW) * 4, fB = (p.fs / WINDOW) * 7;
  const a = [], b = [];
  for (let n = 0; n < WINDOW; n++) {
    const t = n / p.fs;
    const va = 1.55 + 1.2 * Math.sin(2 * Math.PI * fA * t + simPhase) + (Math.random() - 0.5) * 0.03;
    const vb = 1.0 + 0.8 * (((fB * t) % 1) < 0.5 ? 1 : -1) * 0.5 + (Math.random() - 0.5) * 0.03;
    a.push(Math.round(va * 1000));
    b.push(Math.round(vb * 1000));
  }
  simPhase += 0.7;
  return { seq: (simFrame.seq = ((simFrame.seq || 0) + 1) & 0xffff), rate_idx: rateIdx,
           fs_hz: p.fs, window: WINDOW, flags: 0, a, b };
}

// ---- Fuente real HTTP ----------------------------------------------------
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
  chart.data.labels = va.map((_, n) => (n / f.fs_hz * 1000).toFixed(n && f.fs_hz >= 10000 ? 2 : 1));
  chart.data.datasets[0].data = va;
  chart.data.datasets[1].data = vb;
  chart.update();
  fillRow("row-a", va);
  fillRow("row-b", vb);
  const warn = [];
  if (f.flags & 1) warn.push("OVERRUN");
  if (f.flags & 2) warn.push("comando rechazado");
  if (f.flags & 16) warn.push("ADC sin calibrar");
  $("info").textContent =
    `seq ${f.seq} · ${f.fs_hz} Sa/s · ${f.window} muestras · ventana ${(f.window / f.fs_hz * 1000).toFixed(1)} ms` +
    (warn.length ? " · ⚠ " + warn.join(", ") : "") + (f.flags & 8 ? " · modo test" : "") +
    (f.retries ? ` · reintentos SPI: ${f.retries}` : "");
}

// ---- Control y Polling Adaptativo ---------------------------------------
let pollingActive = false;
let isFetching = false;

function setStatus(text, cls) {
  const s = $("status");
  s.textContent = text;
  s.className = "badge " + (cls || "");
}

function updateRefreshOptions(rateIdx) {
  const selRefresh = $("sel-refresh");
  const currentVal = Number(selRefresh.value);
  selRefresh.innerHTML = "";

  if (rateIdx === 0) {
    selRefresh.add(new Option("0.39 Hz (2.56 s)", "2560"));
    selRefresh.add(new Option("0.25 Hz (4 s)", "4000"));
    selRefresh.value = currentVal >= 2560 ? String(currentVal) : "2560";
  } else {
    selRefresh.add(new Option("1 Hz (1 s)", "1000"));
    selRefresh.add(new Option("0.5 Hz (2 s)", "2000"));
    selRefresh.add(new Option("0.25 Hz (4 s)", "4000"));
    selRefresh.value = [1000, 2000, 4000].includes(currentVal) ? String(currentVal) : "1000";
  }
}

async function tick() {
  if (isFetching) return;
  isFetching = true;

  const targetIdx = Number($("sel-rate").value);
  try {
    const f = $("sel-source").value === "sim" ? simFrame(targetIdx) : await apiFrame();

    if ($("sel-source").value === "api" && f.rate_idx !== targetIdx) {
      setStatus("ajustando frecuencia...", "");
      isFetching = false;
      return;
    }

    $("sel-rate").disabled = false;
    render(f);
    setStatus("capturando", "ok");
  } catch (e) {
    setStatus("error: " + e.message, "err");
  } finally {
    isFetching = false;
  }
}

// Bucle adaptativo: calcula cuánto tardó la captura y ajusta el siguiente disparo
async function pollingLoop() {
  while (pollingActive) {
    const startTime = Date.now();
    await tick();
    const elapsedTime = Date.now() - startTime;
    const desiredInterval = Number($("sel-refresh").value);

    // Si la captura tardó más que el intervalo (ej. 2.64 s vs 2.56 s), inicia la siguiente de inmediato (10 ms)
    const delay = Math.max(10, desiredInterval - elapsedTime);
    
    await new Promise((resolve) => setTimeout(resolve, delay));
  }
}

function start() {
  if (pollingActive) return;
  pollingActive = true;
  $("btn-toggle").textContent = "■ Stop";
  $("btn-toggle").classList.add("running");
  pollingLoop();
}

function stop() {
  pollingActive = false;
  $("btn-toggle").textContent = "▶ Start";
  $("btn-toggle").classList.remove("running");
  setStatus("detenido");
}

// ---- Inicialización UI ----------------------------------------------------
PRESETS.forEach((p) => $("sel-rate").add(new Option(p.label, p.idx)));
$("sel-rate").value = 1;
updateRefreshOptions(1);

$("btn-toggle").onclick = () => (pollingActive ? stop() : start());
$("sel-refresh").onchange = () => { if (pollingActive) { stop(); start(); } };
$("sel-source").onchange = () => { if (pollingActive) { stop(); start(); } };

$("sel-rate").onchange = async () => {
  const newIdx = Number($("sel-rate").value);
  updateRefreshOptions(newIdx);

  if ($("sel-source").value === "api") {
    $("sel-rate").disabled = true;
    setStatus("enviando cambio...", "");
    try {
      await apiSetRate(newIdx);
    } catch (e) {
      setStatus("error: " + e.message, "err");
      $("sel-rate").disabled = false;
    }
  }

  if (pollingActive) {
    stop();
    start();
  }
};