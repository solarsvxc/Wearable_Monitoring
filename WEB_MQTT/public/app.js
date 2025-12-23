// ====== Config ======
const MAX_POINTS = 60; // 60 điểm gần nhất
const POLL_MS = 1000; // 1s
const CIRC = 289; // stroke-dasharray cho vòng tròn (r=46)

// ====== DOM ======
const el = {
  bpm: document.getElementById("bpm"),
  spo2: document.getElementById("spo2"),
  temp: document.getElementById("temp"),

  bpmBadge: document.getElementById("bpmBadge"),
  spo2Badge: document.getElementById("spo2Badge"),
  tempBadge: document.getElementById("tempBadge"),

  pillStatus: document.getElementById("pillStatus"),
  lastUpdate: document.getElementById("lastUpdate"),

  dangerValue: document.getElementById("dangerValue"),
  ringProgress: document.getElementById("ringProgress"),
  riskChip: document.getElementById("riskChip"),
  alertText: document.getElementById("alertText"),

  btnClear: document.getElementById("btnClear"),
};

// ====== Helpers ======
function nowTime() {
  const d = new Date();
  const hh = String(d.getHours()).padStart(2, "0");
  const mm = String(d.getMinutes()).padStart(2, "0");
  const ss = String(d.getSeconds()).padStart(2, "0");
  return `${hh}:${mm}:${ss}`;
}

function clamp(x, a, b) {
  return Math.min(b, Math.max(a, x));
}

// phân loại theo danger để đổi màu UI
function riskLevel(danger) {
  if (danger >= 75)
    return { key: "crit", label: "RẤT NGUY HIỂM", color: getCss("--crit") };
  if (danger >= 50)
    return { key: "high", label: "NGUY HIỂM", color: getCss("--high") };
  if (danger >= 25)
    return { key: "warn", label: "CẢNH BÁO", color: getCss("--warn") };
  return { key: "safe", label: "AN TOÀN", color: getCss("--safe") };
}

function getCss(varName) {
  return getComputedStyle(document.documentElement)
    .getPropertyValue(varName)
    .trim();
}

function setStateClass(node, levelKey) {
  node.classList.remove("state-safe", "state-warn", "state-high", "state-crit");
  node.classList.add(`state-${levelKey}`);
}

// đánh giá riêng từng chỉ số (để gắn badge dễ hiểu)
function evalBpm(bpm) {
  if (bpm === 0) return { label: "—", key: "safe" };
  if (bpm < 50) return { label: "Thấp", key: "warn" };
  if (bpm <= 100) return { label: "Ổn", key: "safe" };
  if (bpm <= 120) return { label: "Cao", key: "warn" };
  return { label: "Rất cao", key: "high" };
}

function evalSpo2(spo2) {
  if (spo2 === 0) return { label: "—", key: "safe" };
  if (spo2 >= 95) return { label: "Ổn", key: "safe" };
  if (spo2 >= 92) return { label: "Thấp nhẹ", key: "warn" };
  if (spo2 >= 88) return { label: "Nguy hiểm", key: "high" };
  return { label: "Rất nguy hiểm", key: "crit" };
}

function evalTemp(temp) {
  if (temp === 0) return { label: "—", key: "safe" };
  if (temp < 36.0) return { label: "Hạ nhiệt", key: "warn" };
  if (temp <= 37.5) return { label: "Ổn", key: "safe" };
  if (temp <= 38.5) return { label: "Sốt", key: "warn" };
  return { label: "Sốt cao", key: "high" };
}

// ====== Danger formula (giữ logic gốc nhưng hiển thị tốt hơn) ======
function calcDanger(bpm, spo2, temp) {
  // (giữ y như bạn đang dùng, chỉ clamp)
  let danger = 0;

  if (bpm < 50) danger += (50 - bpm) * 1.5;
  else if (bpm > 100) danger += (bpm - 100) * 1.5;

  if (spo2 < 92) danger += (92 - spo2) * 2;

  if (temp > 37.5) danger += (temp - 37.5) * 10;
  else if (temp < 36) danger += (36 - temp) * 5;

  return clamp(danger, 0, 100);
}

// ====== Chart ======
const ctx = document.getElementById("chart");
const labels = [];
const series = {
  bpm: [],
  spo2: [],
  temp: [],
  danger: [],
};

const chart = new Chart(ctx, {
  type: "line",
  data: {
    labels,
    datasets: [
      {
        label: "BPM",
        data: series.bpm,
        tension: 0.35,
        borderWidth: 2,
        pointRadius: 0,
      },
      {
        label: "SpO₂",
        data: series.spo2,
        tension: 0.35,
        borderWidth: 2,
        pointRadius: 0,
      },
      {
        label: "Temp (°C)",
        data: series.temp,
        tension: 0.35,
        borderWidth: 2,
        pointRadius: 0,
      },
      {
        label: "Danger (%)",
        data: series.danger,
        tension: 0.35,
        borderWidth: 2,
        pointRadius: 0,
      },
    ],
  },
  options: {
    responsive: true,
    maintainAspectRatio: false,
    animation: { duration: 300 },
    plugins: {
      legend: {
        labels: { color: "rgba(231,238,252,0.85)" },
      },
      tooltip: {
        mode: "index",
        intersect: false,
      },
    },
    interaction: { mode: "index", intersect: false },
    scales: {
      x: {
        ticks: { color: "rgba(231,238,252,0.6)" },
        grid: { color: "rgba(255,255,255,0.07)" },
      },
      y: {
        ticks: { color: "rgba(231,238,252,0.6)" },
        grid: { color: "rgba(255,255,255,0.07)" },
      },
    },
  },
});

// clear chart button
el.btnClear.addEventListener("click", () => {
  labels.length = 0;
  Object.keys(series).forEach((k) => (series[k].length = 0));
  chart.update();
});

// ====== UI update ======
function updateGauge(danger) {
  const lvl = riskLevel(danger);

  el.dangerValue.textContent = `${Math.round(danger)}%`;
  el.riskChip.textContent = lvl.label;

  // ring progress
  // dashoffset = full - progress
  const offset = CIRC - (danger / 100) * CIRC;
  el.ringProgress.style.strokeDasharray = String(CIRC);
  el.ringProgress.style.strokeDashoffset = String(offset);
  el.ringProgress.style.stroke = lvl.color;

  // apply state border to gauge card
  const gaugeCard = document.querySelector(".gauge-card");
  setStateClass(gaugeCard, lvl.key);

  // alert text
  const msg =
    lvl.key === "safe"
      ? "Chỉ số đang trong vùng an toàn. Tiếp tục theo dõi."
      : lvl.key === "warn"
      ? "Có dấu hiệu bất thường nhẹ. Nên kiểm tra lại / nghỉ ngơi."
      : lvl.key === "high"
      ? "Mức nguy hiểm cao. Cần theo dõi sát và xử lý kịp thời."
      : "Rất nguy hiểm! Ưu tiên can thiệp ngay và liên hệ hỗ trợ y tế nếu cần.";
  el.alertText.textContent = msg;

  // apply state border to alert card
  const alertCard = document.querySelector(".alert");
  setStateClass(alertCard, lvl.key);
}

function updateBadges(bpm, spo2, temp) {
  const b = evalBpm(bpm);
  const s = evalSpo2(spo2);
  const t = evalTemp(temp);

  el.bpmBadge.textContent = b.label;
  el.spo2Badge.textContent = s.label;
  el.tempBadge.textContent = t.label;

  // màu badge theo trạng thái
  setStateClass(el.bpmBadge, b.key);
  setStateClass(el.spo2Badge, s.key);
  setStateClass(el.tempBadge, t.key);
}

// ====== Fetch loop ======
async function fetchData() {
  try {
    const res = await fetch("/data", { cache: "no-store" });
    if (!res.ok) throw new Error(`HTTP ${res.status}`);
    const data = await res.json();

    // parse numbers
    let bpm = Number(data.bpm) || 0;
    const spo2 = Number(data.spo2) || 0;
    const temp = Number(data.temp) || 0;

    // giữ nguyên behavior cũ: bpm max 110 (nếu bạn muốn bỏ giới hạn, nói mình)
    if (bpm > 110) bpm = 110;

    // update values
    el.bpm.textContent = bpm;
    el.spo2.textContent = spo2;
    el.temp.textContent = temp;

    updateBadges(bpm, spo2, temp);

    const danger = calcDanger(bpm, spo2, temp);
    updateGauge(danger);

    // chart push
    labels.push(nowTime());
    series.bpm.push(bpm);
    series.spo2.push(spo2);
    series.temp.push(temp);
    series.danger.push(Math.round(danger));

    // keep max points
    if (labels.length > MAX_POINTS) {
      labels.shift();
      Object.keys(series).forEach((k) => series[k].shift());
    }
    chart.update();

    // connection UI
    el.pillStatus.textContent = "Kết nối OK";
    el.pillStatus.classList.remove("bad");
    el.pillStatus.classList.add("ok");
    el.lastUpdate.textContent = nowTime();
  } catch (e) {
    console.log("Fetch error:", e);
    el.pillStatus.textContent = "Mất kết nối / Lỗi dữ liệu";
    el.pillStatus.classList.remove("ok");
    el.pillStatus.classList.add("bad");
  }
}

setInterval(fetchData, POLL_MS);
fetchData();
