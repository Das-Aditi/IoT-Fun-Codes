// Global State
let testRunning = false;
let activeTrial = null;
let trialTimer = null;
let roadAnimId = null;
let currentDriverName = "";

let preTestPeakTremor = 0.0;
let hasCalibrated = false;

const TOTAL_TRIALS = 6;
const TRIAL_DURATION_MS = 7000;
let completedTrials = 0;
let sessionLog = {
  driverName: "",
  bpms: [],
  tremors: [],
  trials: []
};

let historyData = [];
let currentPage = 1;
const recordsPerPage = 5;

const dashboardView = document.getElementById("dashboard-view");
const modalSimulation = document.getElementById("simulation-modal");
const modalResults = document.getElementById("results-modal");
const countdownOverlay = document.getElementById("countdown-overlay");
const countdownNumber = document.getElementById("countdown-number");
const countdownPhase = document.getElementById("countdown-phase");

const inputDriverName = document.getElementById("driverName");
const nameError = document.getElementById("nameError");

const btnCalibrate = document.getElementById("btn-calibrate-tremor");
const calibBtnText = document.getElementById("calib-btn-text");
const badgePretestTremor = document.getElementById("badge-pretest-tremor");
const badgePretestStatus = document.getElementById("badge-pretest-status");
const calibProgressWrap = document.getElementById("calib-inline-progress-wrap");
const calibProgressFill = document.getElementById("calib-inline-progress-fill");

const hudHR = document.getElementById("hud-hr-val");
const hudST = document.getElementById("hud-st-val");
const hudBox = document.getElementById("hud-stimulus-box");
const hudText = document.getElementById("stimulus-text");
const hudSwatch = document.getElementById("stimulus-swatch");
const hudSpeaker = document.getElementById("hud-speaker-box");

const btnAccept = document.getElementById("btn-accept");
const btnReject = document.getElementById("btn-reject");

document.getElementById("btn-page-reload")?.addEventListener("click", () => {
  window.location.reload();
});

let audioCtx = null;

function unlockAudio() {
  try {
    if (!audioCtx) {
      const AudioContextClass = window.AudioContext || window.webkitAudioContext;
      if (AudioContextClass) audioCtx = new AudioContextClass();
    }
    if (audioCtx && audioCtx.state === "suspended") {
      audioCtx.resume();
    }
  } catch (e) {
    console.error("Audio init error", e);
  }
}

function playCountdownBeep(isFinal = false) {
  try {
    unlockAudio();
    if (!audioCtx) return;

    const osc = audioCtx.createOscillator();
    const gain = audioCtx.createGain();

    osc.type = isFinal ? "triangle" : "sine";
    osc.frequency.setValueAtTime(isFinal ? 880 : 440, audioCtx.currentTime);

    if (isFinal) {
      osc.frequency.exponentialRampToValueAtTime(1760, audioCtx.currentTime + 0.3);
    }

    gain.gain.setValueAtTime(0.2, audioCtx.currentTime);
    gain.gain.exponentialRampToValueAtTime(0.001, audioCtx.currentTime + (isFinal ? 0.35 : 0.18));

    osc.connect(gain);
    gain.connect(audioCtx.destination);

    osc.start();
    osc.stop(audioCtx.currentTime + (isFinal ? 0.4 : 0.2));
  } catch (e) {
    // Audio fallback
  }
}

function speakColor(colorName) {
  if ("speechSynthesis" in window) {
    window.speechSynthesis.cancel();
    const u = new SpeechSynthesisUtterance(colorName);
    u.rate = 1.15;
    if (hudSpeaker) hudSpeaker.classList.add("active");
    u.onend = () => { if (hudSpeaker) hudSpeaker.classList.remove("active"); };
    u.onerror = () => { if (hudSpeaker) hudSpeaker.classList.remove("active"); };
    window.speechSynthesis.speak(u);
  }
}

const canvas = document.getElementById("roadCanvas");
const ctx = canvas ? canvas.getContext("2d") : null;
let roadOffset = 0;
let roadSpeed = 3.5;

let traffic = [
  { type: "auto",   lane: "left",  progress: 0.20, speed: 0.0045, color: "#15803d" },
  { type: "car",    lane: "right", progress: 0.55, speed: 0.0065, color: "#2563eb" },
  { type: "scooty", lane: "left",  progress: 0.78, speed: 0.0055, color: "#e11d48" }
];

function resizeCanvas() {
  if (!canvas) return;
  canvas.width = window.innerWidth;
  canvas.height = window.innerHeight;
}
window.addEventListener("resize", resizeCanvas);
resizeCanvas();

let selectedMode = 1;
const modeRadios = document.querySelectorAll('input[name="assessmentMode"]');
modeRadios.forEach(radio => {
  radio.addEventListener("change", (e) => {
    if (e.target.checked) {
      selectedMode = parseInt(e.target.value, 10) || 1;
    }
  });
});

function drawIndianVehicle(v, horizon, h, w) {
  const p = Math.max(0.01, Math.min(1.0, v.progress));
  const y = horizon + (h - horizon) * p;
  
  const roadLeftX = (w * 0.46) + ((w * 0.06) - (w * 0.46)) * p;
  const roadRightX = (w * 0.54) + ((w * 0.94) - (w * 0.54)) * p;
  const roadCenterX = (roadLeftX + roadRightX) / 2;

  const laneCenter = (v.lane === "left") 
    ? (roadLeftX + roadCenterX) / 2 
    : (roadCenterX + roadRightX) / 2;

  const baseScale = Math.pow(p, 0.95) * 4.8;
  if (baseScale < 0.2) return;

  ctx.save();
  ctx.translate(laneCenter, y);

  if (p > 0.80) {
    ctx.globalAlpha = Math.max(0, 1 - ((p - 0.80) / 0.15));
  }

  ctx.fillStyle = "rgba(0, 0, 0, 0.35)";
  ctx.beginPath();
  ctx.ellipse(0, 4 * baseScale, 36 * baseScale, 10 * baseScale, 0, 0, Math.PI * 2);
  ctx.fill();

  const isBlinkerOn = Math.floor(Date.now() / 350) % 2 === 0;

  if (v.type === "auto") {
    const bw = 54 * baseScale;
    const bh = 58 * baseScale;

    ctx.fillStyle = "#020617";
    ctx.fillRect(-bw * 0.50, -bh * 0.12, bw * 0.20, bh * 0.25);
    ctx.fillRect(bw * 0.30, -bh * 0.12, bw * 0.20, bh * 0.25);

    ctx.fillStyle = "#15803d";
    ctx.fillRect(-bw * 0.50, -bh * 0.52, bw, bh * 0.44);

    ctx.fillStyle = "#ff1e1e";
    ctx.fillRect(-bw * 0.46, -bh * 0.40, bw * 0.20, bh * 0.14);
    ctx.fillRect(bw * 0.26, -bh * 0.40, bw * 0.20, bh * 0.14);

    ctx.fillStyle = "#facc15";
    ctx.fillRect(-bw * 0.16, -bh * 0.36, bw * 0.32, bh * 0.10);

    ctx.fillStyle = "#090d16";
    ctx.fillRect(-bw * 0.42, -bh * 0.86, bw * 0.84, bh * 0.38);

    ctx.fillStyle = "#ca8a04";
    ctx.beginPath();
    ctx.arc(0, -bh * 0.65, bw * 0.18, 0, Math.PI * 2);
    ctx.fill();

    ctx.fillStyle = "#eab308";
    ctx.beginPath();
    ctx.moveTo(-bw * 0.52, -bh * 0.84);
    ctx.quadraticCurveTo(0, -bh * 1.10, bw * 0.52, -bh * 0.84);
    ctx.lineTo(bw * 0.46, -bh * 0.74);
    ctx.lineTo(-bw * 0.46, -bh * 0.74);
    ctx.closePath();
    ctx.fill();

  } else if (v.type === "car") {
    const cw = 78 * baseScale;
    const ch = 54 * baseScale;

    ctx.fillStyle = "#020617";
    ctx.fillRect(-cw * 0.50, -ch * 0.12, cw * 0.22, ch * 0.26);
    ctx.fillRect(cw * 0.28, -ch * 0.12, cw * 0.22, ch * 0.26);

    ctx.fillStyle = "#0f172a";
    ctx.beginPath();
    ctx.moveTo(-cw * 0.40, -ch * 0.56);
    ctx.lineTo(-cw * 0.30, -ch * 0.98);
    ctx.lineTo(cw * 0.30, -ch * 0.98);
    ctx.lineTo(cw * 0.40, -ch * 0.56);
    ctx.closePath();
    ctx.fill();

    ctx.fillStyle = "#38bdf8";
    ctx.beginPath();
    ctx.moveTo(-cw * 0.36, -ch * 0.58);
    ctx.lineTo(-cw * 0.27, -ch * 0.94);
    ctx.lineTo(cw * 0.27, -ch * 0.94);
    ctx.lineTo(cw * 0.36, -ch * 0.58);
    ctx.closePath();
    ctx.fill();

    ctx.fillStyle = v.color || "#dc2626";
    ctx.fillRect(-cw * 0.50, -ch * 0.58, cw, ch * 0.50);

    ctx.fillStyle = "#ff1a1a";
    ctx.fillRect(-cw * 0.48, -ch * 0.48, cw * 0.24, ch * 0.18);
    ctx.fillRect(cw * 0.24, -ch * 0.48, cw * 0.24, ch * 0.18);

    if (isBlinkerOn) {
      ctx.fillStyle = "#facc15";
      ctx.fillRect(-cw * 0.48, -ch * 0.30, cw * 0.10, ch * 0.10);
      ctx.fillRect(cw * 0.38, -ch * 0.30, cw * 0.10, ch * 0.10);
    }

    ctx.fillStyle = "#ffffff";
    ctx.fillRect(-cw * 0.18, -ch * 0.32, cw * 0.36, ch * 0.12);

  } else {
    const sw = 30 * baseScale;
    const sh = 66 * baseScale;

    ctx.fillStyle = "#020617";
    ctx.fillRect(-sw * 0.20, -sh * 0.14, sw * 0.40, sw * 0.35);

    ctx.fillStyle = v.color || "#e11d48";
    ctx.fillRect(-sw * 0.48, -sh * 0.46, sw * 0.96, sh * 0.36);

    ctx.fillStyle = "#ef4444";
    ctx.fillRect(-sw * 0.25, -sh * 0.40, sw * 0.50, sh * 0.16);

    ctx.fillStyle = "#334155";
    ctx.fillRect(-sw * 0.48, -sh * 0.76, sw * 0.96, sh * 0.34);

    ctx.fillStyle = "#f8fafc";
    ctx.beginPath();
    ctx.arc(0, -sh * 0.90, sw * 0.44, 0, Math.PI * 2);
    ctx.fill();

    ctx.fillStyle = "#090d16";
    ctx.fillRect(-sw * 0.36, -sh * 0.94, sw * 0.72, sh * 0.14);
  }

  ctx.restore();
}

function updateAndDrawTraffic(horizon, h, w) {
  const vehicleTypes = ["auto", "car", "scooty"];
  const carColors = ["#2563eb", "#dc2626", "#ffffff", "#0284c7", "#f59e0b"];
  const lanes = ["left", "right"];

  traffic.sort((a, b) => a.progress - b.progress);

  traffic.forEach((v) => {
    v.progress += v.speed;

    if (v.progress > 0.95) {
      v.progress = 0.05 + Math.random() * 0.08;
      v.type = vehicleTypes[Math.floor(Math.random() * vehicleTypes.length)];
      v.lane = lanes[Math.floor(Math.random() * lanes.length)];
      v.color = carColors[Math.floor(Math.random() * carColors.length)];
      v.speed = 0.0035 + Math.random() * 0.003;
    }

    drawIndianVehicle(v, horizon, h, w);
  });
}

function drawDaylightIndianRoad() {
  if (!modalSimulation || !modalSimulation.classList.contains("modal-active") || !ctx) return;

  const w = canvas.width;
  const h = canvas.height;
  const horizon = h * 0.44;

  ctx.clearRect(0, 0, w, h);

  const skyGrad = ctx.createLinearGradient(0, 0, 0, horizon);
  skyGrad.addColorStop(0, "#2563eb");
  skyGrad.addColorStop(0.65, "#60a5fa");
  skyGrad.addColorStop(1, "#bfdbfe");
  ctx.fillStyle = skyGrad;
  ctx.fillRect(0, 0, w, horizon);

  const sunGrad = ctx.createRadialGradient(w * 0.28, horizon * 0.35, 4, w * 0.28, horizon * 0.35, 60);
  sunGrad.addColorStop(0, "#fffbeb");
  sunGrad.addColorStop(0.3, "rgba(254, 240, 138, 0.8)");
  sunGrad.addColorStop(1, "rgba(254, 240, 138, 0)");
  ctx.fillStyle = sunGrad;
  ctx.fillRect(w * 0.28 - 60, horizon * 0.35 - 60, 120, 120);

  ctx.fillStyle = "#1e3a2f";
  ctx.beginPath();
  ctx.moveTo(0, horizon);
  ctx.quadraticCurveTo(w * 0.25, horizon - 22, w * 0.5, horizon);
  ctx.quadraticCurveTo(w * 0.75, horizon - 28, w, horizon);
  ctx.lineTo(w, horizon + 5);
  ctx.lineTo(0, horizon + 5);
  ctx.closePath();
  ctx.fill();

  const grassGrad = ctx.createLinearGradient(0, horizon, 0, h);
  grassGrad.addColorStop(0, "#15803d");
  grassGrad.addColorStop(1, "#166534");
  ctx.fillStyle = grassGrad;
  ctx.fillRect(0, horizon, w, h - horizon);

  const roadGrad = ctx.createLinearGradient(0, horizon, 0, h);
  roadGrad.addColorStop(0, "#475569");
  roadGrad.addColorStop(1, "#1e293b");
  ctx.fillStyle = roadGrad;
  ctx.beginPath();
  ctx.moveTo(w * 0.46, horizon);
  ctx.lineTo(w * 0.54, horizon);
  ctx.lineTo(w * 0.94, h);
  ctx.lineTo(w * 0.06, h);
  ctx.closePath();
  ctx.fill();

  ctx.strokeStyle = "#ffffff";
  ctx.lineWidth = 4;
  ctx.beginPath();
  ctx.moveTo(w * 0.46, horizon);
  ctx.lineTo(w * 0.06, h);
  ctx.moveTo(w * 0.54, horizon);
  ctx.lineTo(w * 0.94, h);
  ctx.stroke();

  roadOffset = (roadOffset + roadSpeed) % 65;
  ctx.strokeStyle = "#facc15";
  ctx.lineWidth = 5;
  ctx.setLineDash([30, 25]);
  ctx.lineDashOffset = -roadOffset;

  ctx.beginPath();
  ctx.moveTo(w * 0.50, horizon);
  ctx.lineTo(w * 0.50, h);
  ctx.stroke();
  ctx.setLineDash([]);

  updateAndDrawTraffic(horizon, h, w);

  roadAnimId = requestAnimationFrame(drawDaylightIndianRoad);
}
btnCalibrate?.addEventListener("click", () => {
  unlockAudio();
  preTestPeakTremor = 0.0;
  hasCalibrated = false;

  btnCalibrate.disabled = true;
  if (calibProgressWrap) calibProgressWrap.style.display = "block";
  if (calibProgressFill) calibProgressFill.style.width = "0%";
  if (badgePretestStatus) {
    badgePretestStatus.innerText = "(Measuring Steadiness...)";
    badgePretestStatus.style.color = "#38bdf8";
  }

  let timeLeft = 5;
  calibBtnText.innerText = `TESTING (${timeLeft}s)...`;
  playCountdownBeep(false);

  const sampleIntake = setInterval(() => {
    const currentTremor = (typeof DataService !== "undefined") 
      ? parseFloat(DataService.currentTelemetry.tremorDelta) 
      : 0.0;
    
    if (currentTremor > preTestPeakTremor) {
      preTestPeakTremor = currentTremor;
      if (badgePretestTremor) {
        badgePretestTremor.innerText = `${preTestPeakTremor.toFixed(2)} m/s²`;
      }
    }
  }, 50);

  const intakeTimer = setInterval(() => {
    timeLeft--;
    const percent = Math.round(((5 - timeLeft) / 5) * 100);
    if (calibProgressFill) calibProgressFill.style.width = `${percent}%`;

    if (timeLeft > 0) {
      calibBtnText.innerText = `TESTING (${timeLeft}s)...`;
      playCountdownBeep(false);
    } else {
      clearInterval(intakeTimer);
      clearInterval(sampleIntake);
      btnCalibrate.disabled = false;
      calibBtnText.innerText = "RE-TEST (5s)";
      hasCalibrated = true;
      playCountdownBeep(true);

      if (badgePretestTremor) {
        badgePretestTremor.innerText = `${preTestPeakTremor.toFixed(2)} m/s²`;
      }
      if (badgePretestStatus) {
        if (preTestPeakTremor <= 0.50) {
          badgePretestStatus.innerText = "(Optimal Steady Baseline)";
          badgePretestStatus.style.color = "var(--accent-teal)";
        } else {
          badgePretestStatus.innerText = "(Elevated Tremor Detected)";
          badgePretestStatus.style.color = "#ef4444";
        }
      }

      setTimeout(() => {
        if (calibProgressWrap) calibProgressWrap.style.display = "none";
      }, 500);
    }
  }, 1000);
});

document.getElementById("btn-start-test")?.addEventListener("click", (e) => {
  const name = inputDriverName ? inputDriverName.value.trim() : "";
  if (!name) {
    if (nameError) nameError.style.display = "block";
    if (inputDriverName) inputDriverName.focus();
    return;
  }
  if (nameError) nameError.style.display = "none";
  currentDriverName = name;

  if (e && e.target) e.target.blur();
  if (document.activeElement) document.activeElement.blur();

  const checkedRadio = document.querySelector('input[name="assessmentMode"]:checked');
  if (checkedRadio) {
    selectedMode = parseInt(checkedRadio.value, 10) || 1;
  }

  unlockAudio();

  const hrCircle = document.getElementById("hud-hr-circle");
  const stCircle = document.getElementById("hud-st-circle");

  if (selectedMode === 1) {
    if (hrCircle) hrCircle.style.display = "none";
    if (stCircle) stCircle.style.display = "none";
  } else if (selectedMode === 2) {
    if (hrCircle) hrCircle.style.display = "flex";
    if (stCircle) stCircle.style.display = "none";
  } else {
    if (hrCircle) hrCircle.style.display = "flex";
    if (stCircle) stCircle.style.display = "flex";
  }

  modalSimulation.classList.add("modal-active");
  cancelAnimationFrame(roadAnimId);
  drawDaylightIndianRoad();

  countdownOverlay.style.display = "flex";
  let count = 10;
  countdownNumber.innerText = count;
  countdownPhase.innerText = "GET READY";
  playCountdownBeep(false);

  const countTimer = setInterval(() => {
    count--;
    if (count > 0) {
      countdownNumber.innerText = count;

      if (count > 6) {
        countdownPhase.innerText = "GET";
        countdownPhase.style.color = "#38bdf8";
      } else if (count > 3) {
        countdownPhase.innerText = "SET";
        countdownPhase.style.color = "#facc15";
      } else {
        countdownPhase.innerText = "READY";
        countdownPhase.style.color = "#fb923c";
      }

      playCountdownBeep(false);
    } else {
      clearInterval(countTimer);
      countdownNumber.innerText = "GO!";
      countdownPhase.innerText = "STARTING!";
      countdownPhase.style.color = "#22c55e";
      playCountdownBeep(true);

      setTimeout(() => {
        countdownOverlay.style.display = "none";
        startAssessment();
      }, 700);
    }
  }, 1000);
});

let sessionPeakTremor = 0.0;

function startAssessment() {
  testRunning = true;
  completedTrials = 0;

  sessionPeakTremor = preTestPeakTremor || 0.0;

  sessionLog = {
    driverName: currentDriverName,
    bpms: [],
    tremors: [],
    trials: []
  };

  if (hudST) {
    hudST.innerText = sessionPeakTremor.toFixed(2);
  }

  const vitalsPoller = setInterval(() => {
    if (!testRunning) {
      clearInterval(vitalsPoller);
      return;
    }
    const bpm = (typeof DataService !== "undefined") ? DataService.currentTelemetry.bpm : 0;
    const rawIR = (typeof DataService !== "undefined") ? DataService.currentTelemetry.rawIR : 0;
    const currentTremor = (typeof DataService !== "undefined") ? parseFloat(DataService.currentTelemetry.tremorDelta) : 0;

    // Heart rate tracking
    if (bpm > 0) {
      sessionLog.bpms.push(bpm);
      if (hudHR) hudHR.innerText = bpm;
    } else if (rawIR > 25000) {
      if (hudHR) hudHR.innerText = "READ";
    } else {
      if (hudHR) hudHR.innerText = "--";
    }

    if (currentTremor > 0) {
      sessionLog.tremors.push(currentTremor);
      if (currentTremor > sessionPeakTremor) {
        sessionPeakTremor = currentTremor;
      }
    }

    if (hudST) {
      hudST.innerText = sessionPeakTremor.toFixed(2);
    }
  }, 300);

  runTrialStimulus();
}

function runTrialStimulus() {
  if (!testRunning || completedTrials >= TOTAL_TRIALS) {
    concludeAssessment();
    return;
  }

  const colors = (typeof CONFIG !== "undefined" && CONFIG.colors) ? CONFIG.colors : [
    { name: "RED", hex: "#ef4444" },
    { name: "BLUE", hex: "#3b82f6" },
    { name: "GREEN", hex: "#22c55e" },
    { name: "YELLOW", hex: "#eab308" }
  ];

  const visual = colors[Math.floor(Math.random() * colors.length)];
  const isMatch = Math.random() > 0.45;
  const audio = isMatch ? visual : colors[Math.floor(Math.random() * colors.length)];

  activeTrial = {
    visualName: visual.name,
    audioName: audio.name,
    isMatch: (visual.name === audio.name),
    startTime: performance.now(),
    settled: false
  };

  if (hudSwatch) hudSwatch.style.backgroundColor = visual.hex;
  if (hudText) hudText.innerText = "Color: " + visual.name;
  if (hudBox) hudBox.classList.add("active");

  speakColor(audio.name);

  clearTimeout(trialTimer);
  trialTimer = setTimeout(() => {
    if (activeTrial && !activeTrial.settled) {
      const correct = !activeTrial.isMatch;
      sessionLog.trials.push({
        matched: activeTrial.isMatch,
        action: "NONE",
        rt: 7000,
        correct: correct
      });
      activeTrial = null;
      if (hudBox) hudBox.classList.remove("active");
      completedTrials++;
      setTimeout(runTrialStimulus, 1000);
    }
  }, TRIAL_DURATION_MS);
}

function registerDriverChoice(choice) {
  if (!activeTrial || activeTrial.settled) return;
  activeTrial.settled = true;
  clearTimeout(trialTimer);

  const rt = Math.round(performance.now() - activeTrial.startTime);
  const correct = (choice === "AC" && activeTrial.isMatch) || (choice === "RJ" && !activeTrial.isMatch);

  sessionLog.trials.push({
    matched: activeTrial.isMatch,
    action: choice,
    rt: rt,
    correct: correct
  });

  const targetBtn = choice === "AC" ? btnAccept : btnReject;
  if (targetBtn) {
    targetBtn.classList.add("btn-pressed");
    setTimeout(() => targetBtn.classList.remove("btn-pressed"), 180);
  }

  if (hudBox) hudBox.classList.remove("active");
  activeTrial = null;
  completedTrials++;

  setTimeout(runTrialStimulus, 1000);
}

btnAccept?.addEventListener("click", () => registerDriverChoice("AC"));
btnReject?.addEventListener("click", () => registerDriverChoice("RJ"));

window.addEventListener("keydown", (e) => {
  if (!testRunning) return;

  if (e.code === "KeyM") {
    e.preventDefault();
    registerDriverChoice("AC");
  } else if (e.code === "KeyN") {
    e.preventDefault();
    registerDriverChoice("RJ");
  }
});

async function concludeAssessment() {
  testRunning = false;
  clearTimeout(trialTimer);
  cancelAnimationFrame(roadAnimId);
  modalSimulation.classList.remove("modal-active");

  const correctTrialsCount = sessionLog.trials.filter(t => t.correct).length;
  const falseTriggersCount = sessionLog.trials.filter(t => !t.correct && t.action === "AC").length;
  const reactedTrials = sessionLog.trials.filter(t => t.action !== "NONE" && !isNaN(t.rt));

  let meanRT_ms = 0;
  let meanRT_sec = "--";
  if (reactedTrials.length > 0) {
    meanRT_ms = Math.round(reactedTrials.reduce((a, b) => a + b.rt, 0) / reactedTrials.length);
    meanRT_sec = (meanRT_ms / 1000).toFixed(2);
  }

  let psychomotorPassed = false;
  let rtStatusText = "";
  if (correctTrialsCount >= 4 && reactedTrials.length > 0 && parseFloat(meanRT_sec) <= 3.50) {
    psychomotorPassed = true;
    rtStatusText = "✓ High Psychomotor Alertness";
  } else if (correctTrialsCount < 4) {
    psychomotorPassed = false;
    rtStatusText = `⚠ Inattentive: Only ${correctTrialsCount}/6 Correct`;
  } else {
    psychomotorPassed = false;
    rtStatusText = `⚠ Sluggish Reflexes: ${meanRT_sec}s Average`;
  }

  const validBpms = sessionLog.bpms.filter(b => b > 35);
  const avgBpm = validBpms.length > 0 ? Math.round(validBpms.reduce((a, b) => a + b, 0) / validBpms.length) : 0;
  const maxBpm = validBpms.length > 0 ? Math.max(...validBpms) : 0;
  const contactPct = Math.round((validBpms.length / Math.max(1, sessionLog.bpms.length)) * 100);

  let hrPassed = false;
  let hrStatusText = "";
  if (avgBpm === 0 || contactPct < 35) {
    hrPassed = false;
    hrStatusText = "⚠ Telemetry Lost: No Pulse";
  } else if (avgBpm < 55) {
    hrPassed = false;
    hrStatusText = "⚠ Severe Bradycardia / Drowsy";
  } else if (avgBpm > 110) {
    hrPassed = false;
    hrStatusText = "⚠ Tachycardia / Acute Stress";
  } else {
    hrPassed = true;
    hrStatusText = "✓ Normal Resting Rhythm";
  }

  // const validTremors = sessionLog.tremors.filter(t => t > 0);
  // const avgTremor = validTremors.length > 0 ? (sessionLog.tremors.reduce((a, b) => a + b, 0) / validTremors.length).toFixed(2) : "0.00";

  const validTremors = sessionLog.tremors.filter(t => t > 0);
  const avgTremor = validTremors.length > 0 
    ? (sessionLog.tremors.reduce((a, b) => a + b, 0) / validTremors.length).toFixed(2) 
    : "0.00";
  
  // The overall peak recorded outside and inside the simulation
  const overallPeakJerk = Math.max(preTestPeakTremor, sessionPeakTremor).toFixed(2);
  
  const inTestPeakJerk = validTremors.length > 0 ? Math.max(...validTremors) : 0.0;
  // const overallPeakJerk = Math.max(preTestPeakTremor, inTestPeakJerk).toFixed(2);

  let steadinessPassed = false;
  let stStatusText = "";
  let displaySteadinessScore = "--";

  if (parseFloat(overallPeakJerk) === 0 && validTremors.length === 0) {
    steadinessPassed = false;
    stStatusText = "⚠ No Tremor Data / Sensor Offline";
    displaySteadinessScore = "--";
  } else {
    const scoreVal = Math.max(0, Math.min(100, Math.round(100 - (parseFloat(avgTremor) * 22))));
    displaySteadinessScore = scoreVal;

    if (scoreVal >= 70 && parseFloat(avgTremor) <= 1.20 && parseFloat(overallPeakJerk) <= 2.80) {
      steadinessPassed = true;
      stStatusText = "✓ Steady Motor Control";
    } else if (parseFloat(overallPeakJerk) > 2.80) {
      steadinessPassed = false;
      stStatusText = `⚠ Sudden Jerk Spikes (${overallPeakJerk} m/s²)`;
    } else {
      steadinessPassed = false;
      stStatusText = `⚠ Excessive Hand Tremor (${avgTremor} m/s²)`;
    }
  }

  let isOverallFit = false;

  const cardHR = document.getElementById("card-result-hr");
  const cardRT = document.getElementById("card-result-rt");
  const cardST = document.getElementById("card-result-st");
  const resultsGrid = document.getElementById("metrics-results-grid");

  if (selectedMode === 1) {
    isOverallFit = psychomotorPassed;
    if (cardHR) cardHR.style.display = "none";
    if (cardST) cardST.style.display = "none";
    if (cardRT) cardRT.style.display = "flex";
    if (resultsGrid) resultsGrid.className = "metrics-trio-grid results-single-card";
  } else if (selectedMode === 2) {
    isOverallFit = (psychomotorPassed && hrPassed);
    if (cardHR) cardHR.style.display = "flex";
    if (cardST) cardST.style.display = "none";
    if (cardRT) cardRT.style.display = "flex";
    if (resultsGrid) resultsGrid.className = "metrics-trio-grid results-dual-cards";
  } else {
    isOverallFit = (psychomotorPassed && hrPassed && steadinessPassed);
    if (cardHR) cardHR.style.display = "flex";
    if (cardST) cardST.style.display = "flex";
    if (cardRT) cardRT.style.display = "flex";
    if (resultsGrid) resultsGrid.className = "metrics-trio-grid";
  }

  const shortVerdictElem = document.getElementById("res-short-verdict");
  if (shortVerdictElem) {
    shortVerdictElem.innerText = isOverallFit ? "PASS" : "FAIL";
    shortVerdictElem.className = isOverallFit ? "results-short-tag short-tag-pass" : "results-short-tag short-tag-fail";
  }

  const banner = document.getElementById("res-verdict-banner");
  if (banner) {
    banner.className = isOverallFit ? "verdict-banner verdict-pass" : "verdict-banner verdict-fail";
    banner.innerHTML = isOverallFit 
      ? `<span class="icon">✓</span> <span>DRIVER READY: FIT & VIGILANT</span>` 
      : `<span class="icon">⚠</span> <span>DRIVER NOT READY</span>`;
  }

  const meanRtElem = document.getElementById("res-mean-rt");
  if (meanRtElem) {
    meanRtElem.innerText = (reactedTrials.length > 0 && meanRT_sec !== "--") ? meanRT_sec : "--";
    const unitElem = meanRtElem.nextElementSibling;
    if (unitElem && unitElem.classList.contains("unit")) {
      unitElem.style.display = (reactedTrials.length > 0 && meanRT_sec !== "--") ? "inline" : "none";
    }
  }
  document.getElementById("res-correct-trials").innerText = `${correctTrialsCount} / ${TOTAL_TRIALS} (${Math.round((correctTrialsCount / TOTAL_TRIALS) * 100)}%)`;
  document.getElementById("res-false-triggers").innerText = falseTriggersCount;
  const rtElem = document.getElementById("res-rt-status");
  if (rtElem) {
    rtElem.innerText = rtStatusText;
    rtElem.style.color = psychomotorPassed ? "var(--accent-teal)" : "#ef4444";
  }

  document.getElementById("res-avg-hr").innerText = avgBpm > 0 ? avgBpm : "--";
  document.getElementById("res-max-hr").innerText = maxBpm > 0 ? maxBpm + " BPM" : "-- BPM";
  document.getElementById("res-contact-pct").innerText = (avgBpm > 0 ? contactPct : 0) + "%";
  const hrElem = document.getElementById("res-hr-status");
  if (hrElem) {
    hrElem.innerText = hrStatusText;
    hrElem.style.color = hrPassed ? "var(--accent-teal)" : "#ef4444";
  }

  document.getElementById("res-steadiness-score").innerText = displaySteadinessScore;
  document.getElementById("res-peak-jerk").innerText = `${overallPeakJerk} m/s²`;
  document.getElementById("res-auditory-ratio").innerText = `${correctTrialsCount}/${TOTAL_TRIALS} Concordant`;
  const stElem = document.getElementById("res-st-status");
  if (stElem) {
    stElem.innerText = stStatusText;
    stElem.style.color = steadinessPassed ? "var(--accent-teal)" : "#ef4444";
  }

  modalResults.classList.add("modal-active");

  // Save session record
  const record = {
    time: new Date().toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', second: '2-digit' }),
    name: `${sessionLog.driverName} (Mode ${selectedMode})`,
    avgBpm: (selectedMode >= 2 && avgBpm > 0) ? avgBpm + " BPM" : "--",
    rt: reactedTrials.length > 0 ? meanRT_sec + " s" : "--",
    score: `${correctTrialsCount}/${TOTAL_TRIALS}`,
    tremor: (selectedMode === 3) ? `${overallPeakJerk} m/s²` : "--",
    verdict: isOverallFit ? "PASSED" : "FAILED"
  };

  if (typeof DataService !== "undefined") {
    await DataService.saveSession(record);
  }
  await loadHistory();
}

document.getElementById("btn-exit-dashboard")?.addEventListener("click", () => {
  modalResults.classList.remove("modal-active");
});

document.getElementById("btn-save-retest")?.addEventListener("click", () => {
  modalResults.classList.remove("modal-active");
  document.getElementById("btn-start-test")?.click();
});

async function loadHistory() {
  historyData = [];
  const candidateUrls = [
    "../api.php?action=get_history",
    "../app/api.php?action=get_history",
    "/drive_test/api.php?action=get_history",
    "/drive_test/app/api.php?action=get_history",
    "../stats.json",
    "/drive_test/stats.json"
  ];

  for (const url of candidateUrls) {
    try {
      const sep = url.includes("?") ? "&" : "?";
      const res = await fetch(`${url}${sep}_=${Date.now()}`, { cache: "no-store" });
      if (res.ok) {
        const raw = await res.json();
        let parsed = Array.isArray(raw) ? raw : (raw?.data || []);
        if (parsed.length > 0) {
          historyData = parsed;
          break;
        }
      }
    } catch (e) {}
  }
  renderTablePage();
}

function renderTablePage() {
  const tbody = document.getElementById("history-tbody");
  if (!tbody) return;
  tbody.innerHTML = "";

  const totalRecords = historyData.length;
  const totalPages = Math.max(1, Math.ceil(totalRecords / recordsPerPage));

  if (currentPage > totalPages) currentPage = totalPages;
  if (currentPage < 1) currentPage = 1;

  const start = (currentPage - 1) * recordsPerPage;
  const pageItems = historyData.slice(start, start + recordsPerPage);

  if (pageItems.length === 0) {
    tbody.innerHTML = `<tr><td colspan="7" style="text-align:center; color:#64748b; padding:20px;">No previous sessions recorded.</td></tr>`;
  } else {
    pageItems.forEach(row => {
      const tr = document.createElement("tr");
      const isPassed = (row.verdict === "PASSED" || row.verdict === "FIT");
      tr.innerHTML = `
        <td>${row.time || '--'}</td>
        <td><strong>${row.name || 'Unknown'}</strong></td>
        <td>${row.avgBpm || '--'}</td>
        <td>${row.rt || '--'}</td>
        <td>${row.score || '--'}</td>
        <td>${row.tremor || '--'}</td>
        <td style="color:${isPassed ? 'var(--accent-green, #10b981)' : 'var(--accent-red, #ef4444)'}; font-weight:800;">${row.verdict || '--'}</td>
      `;
      tbody.appendChild(tr);
    });
  }

  const indicator = document.getElementById("page-indicator");
  if (indicator) indicator.innerText = `Page ${currentPage} of ${totalPages}`;

  const btnPrev = document.getElementById("btn-prev-page");
  const btnNext = document.getElementById("btn-next-page");

  if (btnPrev) btnPrev.disabled = (currentPage <= 1);
  if (btnNext) btnNext.disabled = (currentPage >= totalPages);
}

document.getElementById("btn-prev-page")?.addEventListener("click", (e) => {
  e.preventDefault();
  if (currentPage > 1) {
    currentPage--;
    renderTablePage();
  }
});

document.getElementById("btn-next-page")?.addEventListener("click", (e) => {
  e.preventDefault();
  const totalPages = Math.ceil(historyData.length / recordsPerPage);
  if (currentPage < totalPages) {
    currentPage++;
    renderTablePage();
  }
});

if (document.readyState === "loading") {
  document.addEventListener("DOMContentLoaded", loadHistory);
} else {
  loadHistory();
}