const DataService = {
  currentTelemetry: {
    isOnline: false,
    heartRate: "No sensor stream",
    bpm: 0,
    rawIR: 0,
    steadiness: "STOPPED",
    tremorDelta: 0.00
  },

  async pollTelemetry() {
    try {
      const res = await fetch(CONFIG.telemetryEndpoint);
      const data = await res.json();

      this.currentTelemetry.isOnline = (data.online === true);
      this.currentTelemetry.heartRate = data.heart_rate || "No finger is detected";

      if (this.currentTelemetry.isOnline && !this.currentTelemetry.heartRate.includes("No")) {
        // Extract raw IR if available
        const irMatch = this.currentTelemetry.heartRate.match(/IR=([0-9]+)/);
        this.currentTelemetry.rawIR = irMatch ? parseInt(irMatch[1]) : 0;

        // Extract BPM (matches BPM=xx or AvgBPM=xx)
        const bpmMatch = this.currentTelemetry.heartRate.match(/(?:Avg)?BPM=([0-9]+)/);
        const parsedBpm = bpmMatch ? parseInt(bpmMatch[1]) : 0;
        
        // If a beat was registered (> 35), show it; otherwise mark as measuring/detecting
        this.currentTelemetry.bpm = parsedBpm;
      } else {
        this.currentTelemetry.bpm = 0;
        this.currentTelemetry.rawIR = 0;
      }

      // Parse MPU-6050 Accelerometer & Filter Glitches
      if (this.currentTelemetry.isOnline && data.steadiness && data.steadiness !== "STOPPED") {
        const ax = parseFloat(data.steadiness.match(/AX=([-\d.]+)/)?.[1] || 0);
        const ay = parseFloat(data.steadiness.match(/AY=([-\d.]+)/)?.[1] || 0);
        const az = parseFloat(data.steadiness.match(/AZ=([-\d.]+)/)?.[1] || 9.8);

        // Discard broken I2C packets where values exceed 30 m/s^2
        if (Math.abs(ax) < 30 && Math.abs(ay) < 30 && Math.abs(az) < 30) {
          const mag = Math.sqrt(ax * ax + ay * ay + az * az);
          this.currentTelemetry.tremorDelta = Math.abs(mag - 9.8).toFixed(2);
        }
      } else {
        this.currentTelemetry.tremorDelta = 0.00;
      }

      // Live Dashboard Navbar Pills Update
      const navHR = document.getElementById("nav-hr");
      const navST = document.getElementById("nav-st");
      const pillMode = document.getElementById("pill-mode");

      if (navHR) {
        if (!this.currentTelemetry.isOnline) {
          navHR.innerText = "OFFLINE";
        } else if (this.currentTelemetry.bpm > 0) {
          navHR.innerText = this.currentTelemetry.bpm + " BPM";
        } else if (this.currentTelemetry.rawIR > 50000) {
          navHR.innerText = "Sensing Pulse...";
        } else {
          navHR.innerText = "Place Finger";
        }
      }

      if (navST) {
        navST.innerText = this.currentTelemetry.isOnline ? this.currentTelemetry.tremorDelta + " m/s²" : "--";
      }

      if (pillMode) {
        pillMode.innerHTML = this.currentTelemetry.isOnline 
          ? `<span style="color:#10b981; font-weight:700;">● ESP32 Online</span>` 
          : `<span style="color:#ef4444; font-weight:700;">○ ESP32 Disconnected</span>`;
      }

    } catch (e) {
      this.currentTelemetry.isOnline = false;
      this.currentTelemetry.bpm = 0;
      this.currentTelemetry.tremorDelta = 0.00;
    }
  },

  async fetchHistory() {
    try {
      const res = await fetch(CONFIG.historyEndpoint + "?_=" + Date.now(), { cache: "no-store" });
      const data = await res.json();
      return Array.isArray(data) ? data : [];
    } catch (e) {
      console.error("fetchHistory error:", e);
      return [];
    }
  },

  async saveSession(record) {
    try {
      await fetch(CONFIG.saveEndpoint, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ action: "save_result", record })
      });
    } catch (e) {
      console.error("Failed to save session", e);
    }
  }
};

// Continuous background polling (every 300ms for responsive dials)
setInterval(() => DataService.pollTelemetry(), 300);