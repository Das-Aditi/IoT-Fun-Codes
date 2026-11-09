// PulseDetector.h - beat detector that works for weak AND strong finger signals.
// Feed it one IR sample at a time at a steady rate (about 100 per second).
// It removes the DC level, smooths the wave, and finds peaks relative to the
// signal's OWN size (so it does not care if the signal is 30 or 3000 counts tall).
#pragma once
#include <math.h>

class PulseDetector {
public:
  // sampleHz = how many samples per second you feed in
  void begin(float sampleHz) {
    fs = sampleHz;
    reset();
  }
  void reset() {
    dc = 0; lp1 = lp2 = 0; prev1 = prev2 = 0; env = 0;
    lastPeakMs = 0; haveDc = false; nIv = 0; bpm = 0; sinceBeatSamples = 0;
  }
  // returns true when a new beat was found in this sample
  bool update(long ir, unsigned long nowMs) {
    if (!haveDc) { dc = ir; haveDc = true; }
    dc += (ir - dc) * (1.0f / (fs * 0.8f));          // slow DC tracker (~0.8 s)
    float ac = (float)ir - dc;
    // two light low-pass stages remove sensor noise (cut-off ~ 5 Hz)
    float a = 1.0f - expf(-2.0f * 3.14159f * 5.0f / fs);
    lp1 += (ac - lp1) * a;
    lp2 += (lp1 - lp2) * a;
    float s = lp2;

    // envelope = recent peak size, decays slowly so it follows signal changes
    float as = fabsf(s);
    if (as > env) env += (as - env) * 0.2f; else env -= env * (1.0f / (fs * 3.0f));
    sinceBeatSamples++;

    // peak = previous sample higher than both neighbours
    bool peak = (prev1 > prev2) && (prev1 >= s) && (prev1 > 0.35f * env) && env > 8.0f;
    prev2 = prev1; prev1 = s;
    if (!peak) return false;

    unsigned long dt = nowMs - lastPeakMs;
    if (lastPeakMs != 0 && dt < 300) return false;   // faster than 200 BPM = ripple, skip
    unsigned long prevPeak = lastPeakMs;
    lastPeakMs = nowMs;
    sinceBeatSamples = 0;
    if (prevPeak == 0 || dt > 1700) return true;     // first beat, or a long gap: just restart timing

    iv[ivPos] = dt; ivPos = (ivPos + 1) % N; if (nIv < N) nIv++;
    // median of the stored intervals ignores one wrong beat
    unsigned long t[N]; for (int i = 0; i < nIv; i++) t[i] = iv[i];
    for (int i = 1; i < nIv; i++) { unsigned long k = t[i]; int j = i - 1; while (j >= 0 && t[j] > k) { t[j+1] = t[j]; j--; } t[j+1] = k; }
    unsigned long med = t[nIv / 2];
    bpm = 60000.0f / (float)med;
    return true;
  }
  // BPM only once at least 3 clean intervals exist and a beat was seen in the last 3 s
  int bpmValue() const {
    if (nIv < 3 || sinceBeatSamples > fs * 3.0f) return 0;
    if (bpm < 40 || bpm > 200) return 0;
    return (int)(bpm + 0.5f);
  }
private:
  static const int N = 5;
  float fs = 100, dc, lp1, lp2, prev1, prev2, env, bpm;
  bool haveDc;
  unsigned long lastPeakMs, iv[N]; int ivPos = 0, nIv = 0;
  float sinceBeatSamples;
};
