const CONFIG = {
  // API endpoints
  telemetryEndpoint: '../api.php?action=get_telemetry',
  historyEndpoint: '../api.php?action=get_history',
  saveEndpoint: '../api.php',

  // Test parameters
  countdownSeconds: 10,       // 10s pre-drive focus countdown
  testDurationSeconds: 40,    // Full test window
  stimulusMinIntervalMs: 3500,
  stimulusMaxIntervalMs: 6500,
  stimulusDisplayWindowMs: 2000,
  
  // Hand tremor thresholds (MPU-6050 delta m/s^2)
  steadinessThresholdWarning: 0.85,
  steadinessThresholdDanger: 1.75,
  
  // Auditory-visual colors
  colors: [
    { name: "RED", hex: "#ef4444" },
    { name: "GREEN", hex: "#10b981" },
    { name: "BLUE", hex: "#3b82f6" },
    { name: "YELLOW", hex: "#f59e0b" }
  ]
};