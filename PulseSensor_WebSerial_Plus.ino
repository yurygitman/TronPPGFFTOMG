// BreathRate firmware — streams "signal,bpm,ibi,beat,amp,inside" for the web dashboard.
// Works on the XIAO ESP32-S3 AND on classic Arduino boards (Uno, Nano, Mega, Leonardo…).
#include <PulseSensorPlayground.h>

PulseSensorPlayground pulseSensor;

// PulseSensor Signal -> analog pin (auto-selected per board)
#if defined(ESP32)
  const int PULSE_PIN = 1;           // XIAO ESP32-S3: GPIO1 (A0 / D0)
#else
  const int PULSE_PIN = A0;          // classic Arduino: A0
#endif
const int THRESHOLD  = 550;          // library beat threshold, 10-bit scale
                                     // (the dashboard's default "Auto" beat detector
                                     //  ignores this and adapts to the live signal)

// "Frame-paced" streaming: a steady, jitter-free sample clock driven by micros()
// instead of delay(). The dashboard auto-measures this rate and decimates to its
// internal 50 Hz DSP, so any rate here is safe. 100 Hz gives a smooth, snappy scope.
const uint32_t SAMPLE_HZ = 100;
const uint32_t SAMPLE_US = 1000000UL / SAMPLE_HZ;

uint32_t nextSampleUs = 0;
char     line[40];

void setup() {
  Serial.begin(115200);
  delay(800);
#if defined(ESP32)
  analogReadResolution(10);          // 0–1023, matches the library + TRIGGER 550
#endif
  pulseSensor.analogInput(PULSE_PIN);
  pulseSensor.setThreshold(THRESHOLD);
  pulseSensor.begin();
  nextSampleUs = micros();
}

void loop() {
  // Hold an exact cadence; if a USB stall makes us fall behind, resync (don't burst).
  uint32_t now = micros();
  if ((int32_t)(now - nextSampleUs) < 0) return;
  nextSampleUs += SAMPLE_US;
  if ((int32_t)(micros() - nextSampleUs) > (int32_t)SAMPLE_US) nextSampleUs = micros() + SAMPLE_US;

  int  signal = pulseSensor.getLatestSample();
  int  bpm    = pulseSensor.getBeatsPerMinute();
  int  ibi    = pulseSensor.getInterBeatIntervalMs();
  bool beat   = pulseSensor.sawStartOfBeat();
  int  amp    = pulseSensor.getPulseAmplitude();   // strength meter
  bool inside = pulseSensor.isInsideBeat();         // filled beat circle

  // 6-field CSV: signal,bpm,ibi,beat,amp,inside  — one write() keeps the line atomic
  int n = snprintf(line, sizeof(line), "%d,%d,%d,%d,%d,%d\n",
                   signal, bpm, ibi, beat ? 1 : 0, amp, inside ? 1 : 0);
  Serial.write((const uint8_t*)line, n);
}
