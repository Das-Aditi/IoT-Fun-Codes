<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Driver Readiness & Vigilance Assessment</title>
  <link rel="stylesheet" href="../inc/style.css">
</head>
<body>

  <header class="navbar">
    <div class="brand">SYSTEM: DRIVER VIGILANCE</div>
    <div class="system-status">
      <div class="pill" id="pill-hr">Pulse: <span id="nav-hr">--</span></div>
      <div class="pill" id="pill-st">Tremor: <span id="nav-st">--</span></div>
      <div class="pill" id="pill-mode">Telemetry: <span>Connecting...</span></div>
      <button class="pill-btn-reload" id="btn-page-reload" title="Reload page and reset telemetry">↻ Refresh</button>
    </div>
  </header>

  <main class="dashboard-container" id="dashboard-view">
    <section class="hero-box">
      <h1 class="hero-title">Driver Psychomotor & Vital Assessment</h1>
      <p class="hero-subtitle">Cognitive vigilance with continuous heart rate monitoring, and hand steadiness diagnostic.</p>
      
      <div class="assessment-console-box">
        
        <div class="console-row">
          <label for="driverName" class="console-label">Name / ID</label>
          <div class="console-input-wrap">
            <input type="text" id="driverName" placeholder="Enter driver name..." autocomplete="off">
            <div id="nameError" class="input-error-msg">Please enter a driver name before starting.</div>
          </div>
        </div>

        <div class="console-row">
          <label class="console-label">Select Mode:</label>
          <div class="mode-options-inline">
            <label class="radio-option">
              <input type="radio" name="assessmentMode" value="1" checked>
              <span class="custom-radio"></span>
              <span class="radio-text">Reflex test</span>
            </label>
            <label class="radio-option">
              <input type="radio" name="assessmentMode" value="2">
              <span class="custom-radio"></span>
              <span class="radio-text">Reflex & Pulse test</span>
            </label>
            <label class="radio-option">
              <input type="radio" name="assessmentMode" value="3">
              <span class="custom-radio"></span>
              <span class="radio-text">Full Diagnostic</span>
            </label>
          </div>
        </div>

        <div class="pretest-intake-box">
          <div class="pretest-row-top">
            <button type="button" class="btn-calibrate-intake" id="btn-calibrate-tremor">
              <span class="calib-btn-icon"></span>
              <span id="calib-btn-text">CALCULATE TREMOR</span>
            </button>
            <div class="pretest-display-badge">
              <span class="badge-title">PEAK:</span>
              <strong id="badge-pretest-tremor">-- m/s²</strong>
              <span class="badge-verdict" id="badge-pretest-status">(5sec Calibration)</span>
            </div>
          </div>

          <div class="calib-inline-progress" id="calib-inline-progress-wrap" style="display: none;">
            <div class="calib-inline-fill" id="calib-inline-progress-fill"></div>
          </div>
        </div>

        <div class="console-action-row">
          <button class="btn-pill-start" id="btn-start-test">
            <svg viewBox="0 0 24 24" fill="currentColor">
              <path d="M8 5v14l11-7z"/>
            </svg>
            <span>START TEST</span>
          </button>
        </div>

      </div>
    </section>

    <section class="rules-card">
      <div class="rules-header">
        <!-- <span class="rules-badge">GUIDELINES</span> -->
        <h2 class="rules-title">Simulation Rules & Instructions</h2>
      </div>
      <div class="rules-grid">
        <div class="rule-item">
          <div class="rule-num">1</div>
          <div class="rule-body">
            <strong>Finger on Sensor</strong>
            <p>Keep finger rested on sensor.</p>
          </div>
        </div>
        <div class="rule-item">
          <div class="rule-num">2</div>
          <div class="rule-body">
            <strong>Listen & Watch</strong>
            <p>Compare spoken and displayed colors.</p>
          </div>
        </div>
        <div class="rule-item">
          <div class="rule-num">3</div>
          <div class="rule-body">
            <strong>Match Condition</strong>
            <p>Press <b>M</b> key on match.</p>
          </div>
        </div>
        <div class="rule-item">
          <div class="rule-num">4</div>
          <div class="rule-body">
            <strong>Mismatch Condition</strong>
            <p>Press <b>N</b> key on mismatch.</p>
          </div>
        </div>
        <div class="rule-item">
          <div class="rule-num">5</div>
          <div class="rule-body">
            <strong>Passing Criteria</strong>
            <p>Pass criteria for selected mode.</p>
          </div>
        </div>
      </div>
    </section>

    <section class="history-section">
      <h2 class="section-heading">Previous Test Statistics</h2>
      <div class="table-wrapper">
        <table id="history-table">
          <thead>
            <tr>
              <th>Timestamp</th>
              <th>Driver Name</th>
              <th>Avg. Heart Rate</th>
              <th>Mean Reaction (s)</th>
              <th>Trials Passed</th>
              <th>Steadiness</th>
              <th>Verdict</th>
            </tr>
          </thead>
          <tbody id="history-tbody">
          </tbody>
        </table>
      </div>

      <div class="pagination-bar">
        <button class="page-btn" id="btn-prev-page" disabled>&larr; Previous</button>
        <span class="page-info" id="page-indicator">Page 1 of 1</span>
        <button class="page-btn" id="btn-next-page" disabled>Next &rarr;</button>
      </div>
    </section>
  </main>

  <div class="fullscreen-modal" id="simulation-modal">
    <div class="countdown-overlay" id="countdown-overlay">
      <div class="countdown-card">
        <div class="countdown-header-title">COUNTDOWN</div>
        <div class="countdown-circle-badge">
          <span class="countdown-digits" id="countdown-number">10</span>
        </div>
        <div class="countdown-phase-tag" id="countdown-phase">GET READY</div>
      </div>
    </div>

    <div class="cockpit-viewport">
      <canvas id="roadCanvas"></canvas>

      <div class="hud-top-bar">
        <div class="hud-circle" id="hud-st-circle" title="Steadiness (MPU-6050)">
          <span class="label">ST</span>
          <span class="val" id="hud-st-val">--</span>
        </div>

        <div class="hud-target-box" id="hud-stimulus-box">
          <span class="target-pip" id="stimulus-swatch"></span>
          <span id="stimulus-text">Color: RED</span>
        </div>

        <div class="hud-circle" id="hud-hr-circle" title="Heart Rate (MAX30102)">
          <span class="label">Pulse</span>
          <span class="val" id="hud-hr-val">--</span>
        </div>
      </div>

      <div class="hud-audio-cue" id="hud-speaker-box">
        <svg width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="#00c9a7" stroke-width="2">
          <polygon points="11 5 6 9 2 9 2 15 6 15 11 19 11 5"></polygon>
          <path d="M19.07 4.93a10 10 0 0 1 0 14.14M15.54 8.46a5 5 0 0 1 0 7.07"></path>
        </svg>
        <div class="cue-bars">
          <span></span><span></span><span></span>
        </div>
      </div>

      <div class="cockpit-rhd-dashboard">
        <div class="center-console">
          <button class="dash-btn btn-ac" id="btn-accept">MATCH (Key M)</button>
          <button class="dash-btn btn-rj" id="btn-reject">MISMATCH (Key N)</button>
        </div>

        <div class="driver-wheel-mount">
          <svg class="steering-wheel" viewBox="0 0 120 120">
            <circle cx="60" cy="60" r="54" fill="none" stroke="#222f3e" stroke-width="8"/>
            <circle cx="60" cy="60" r="50" fill="none" stroke="#353b48" stroke-width="2"/>
            <circle cx="60" cy="60" r="18" fill="#1e272e" stroke="#485460" stroke-width="3"/>
            <line x1="8" y1="60" x2="42" y2="60" stroke="#353b48" stroke-width="6"/>
            <line x1="78" y1="60" x2="112" y2="60" stroke="#353b48" stroke-width="6"/>
            <line x1="60" y1="78" x2="60" y2="114" stroke="#353b48" stroke-width="6"/>
          </svg>
        </div>
      </div>
    </div>
  </div>

  <div class="fullscreen-modal" id="results-modal">
    <div class="results-card-window">
      <h1 class="results-main-title">ASSESSMENT RESULTS</h1>

      <div class="results-short-tag-wrapper">
        <span class="results-short-tag" id="res-short-verdict">FAIL</span>
      </div>

      <div class="verdict-banner" id="res-verdict-banner">
        <span class="icon" id="res-verdict-icon">⚠</span>
        <span id="res-verdict-text">DRIVER NOT READY</span>
      </div>

      <div class="metrics-trio-grid" id="metrics-results-grid">
        
        <div class="trio-card" id="card-result-hr">
          <div class="card-head">
            <span class="card-tag">AVG. HEART RATE</span>
            <div class="badge-icon badge-red">♥</div>
          </div>
          <div class="hero-val-line">
            <span class="big-num" id="res-avg-hr">--</span>
            <span class="unit">BPM</span>
          </div>
          <div class="eval-line" id="res-hr-status">--</div>
          <div class="stat-footer-row">
            <span>MAX RECORDED:</span>
            <strong id="res-max-hr">-- BPM</strong>
          </div>
          <div class="stat-footer-row">
            <span>FINGER CONTACT:</span>
            <strong id="res-contact-pct">--%</strong>
          </div>
        </div>

        <div class="trio-card" id="card-result-rt">
          <div class="card-head">
            <span class="card-tag">QUICK RESPONSE STATS</span>
            <div class="badge-icon badge-amber">⏱</div>
          </div>
          <div class="hero-val-line">
            <span class="big-num" id="res-mean-rt">--</span>
            <span class="unit">s</span>
          </div>
          <div class="eval-line" id="res-rt-status">--</div>
          <div class="stat-footer-row">
            <span>CORRECT DECISIONS:</span>
            <strong id="res-correct-trials">0 / 6 (0%)</strong>
          </div>
          <div class="stat-footer-row">
            <span>FALSE TRIGGERS:</span>
            <strong id="res-false-triggers">0</strong>
          </div>
        </div>

        <div class="trio-card" id="card-result-st">
          <div class="card-head">
            <span class="card-tag">SOUND & STEADINESS</span>
            <div class="badge-icon badge-cyan">〰</div>
          </div>
          <div class="hero-val-line">
            <span class="big-num" id="res-steadiness-score">--</span>
            <span class="unit">/ 100</span>
          </div>
          <div class="eval-line" id="res-st-status">--</div>
          <div class="stat-footer-row">
            <span>PEAK TREMOR:</span>
            <strong id="res-peak-jerk">-- m/s²</strong>
          </div>
          <div class="stat-footer-row">
            <span>AUDITORY BINDING:</span>
            <strong id="res-auditory-ratio">-- Concordant</strong>
          </div>
        </div>

      </div>

      <div class="results-actions">
        <button class="btn-res-save" id="btn-save-retest">
          <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5">
            <path d="M21.5 2v6h-6M21.34 15.57a10 10 0 1 1-.57-8.38l5.67-5.67"/>
          </svg>
          SAVE & RETEST
        </button>
        <button class="btn-res-exit" id="btn-exit-dashboard">EXIT TO DASHBOARD</button>
      </div>
    </div>
  </div>

  <script src="../inc/settings.js"></script>
  <script src="../app/data.js"></script>
  <script src="ui.js"></script>
</body>
</html>