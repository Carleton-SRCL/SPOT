#include <Arduino.h>
#include <math.h>
#include <string.h>

// =========================================================
// Hardware configuration
// =========================================================
const int THRUSTER_PINS[8] = {0, 1, 2, 3, 4, 5, 6, 7};
const int AIR_BEARING_PIN  = 8;
const int STATUS_LED_PIN   = 13;

const int NUM_THRUSTERS = 8;

// =========================================================
// PWM configuration 
// =========================================================
const float         PWM_FREQUENCY  = 5.0f;   
const unsigned long PWM_PERIOD_MS  =
    (unsigned long)(1000.0f / PWM_FREQUENCY + 0.5f);

// =========================================================
// Serial protocol
// =========================================================
const uint8_t SYNC_BYTE = 0xAA;

// 8 thrusters + 1 puck signal
const size_t NUM_FLOATS    = 9;
const size_t PAYLOAD_BYTES = NUM_FLOATS * 4;

// =========================================================
// Watchdog
// =========================================================
const unsigned long PACKET_TIMEOUT_MS = 2000;

// =========================================================
// State
// =========================================================
float currentDuty[8] = {0};
bool  currentPuck    = false;

uint8_t rxBuffer[PAYLOAD_BYTES];
size_t  rxIndex = 0;

enum {
  WAITING_SYNC,
  READING_PAYLOAD
} rxState = WAITING_SYNC;

unsigned long lastPacketTime  = 0;
bool          simulinkActive  = false;

unsigned long pwmCycleStart_ms = 0;

// =========================================================
// Thruster duty update
// =========================================================
void setThrusterDuty(int idx, float pct) {

  if (isnan(pct) || pct < 0.0f)
    pct = 0.0f;

  if (pct > 100.0f)
    pct = 100.0f;

  currentDuty[idx] = pct;

}

// =========================================================
// PWM update (call every loop iteration)
// =========================================================
void updateSoftwarePWM() {

  unsigned long now     = millis();
  unsigned long elapsed = now - pwmCycleStart_ms;

  if (elapsed >= PWM_PERIOD_MS) {

    // Roll forward in whole periods so a long stall doesn't desync
    pwmCycleStart_ms += (elapsed / PWM_PERIOD_MS) * PWM_PERIOD_MS;
    elapsed = now - pwmCycleStart_ms;
  }

  for (int i = 0; i < NUM_THRUSTERS; i++) {

    float pct = currentDuty[i];
    bool  on;

    if (pct <= 0.0f) {
      on = false;
    } else if (pct >= 100.0f) {
      on = true;
    } else {

      unsigned long onTime_ms =
          (unsigned long)((pct / 100.0f) * PWM_PERIOD_MS);

      on = (elapsed < onTime_ms);
    }

    digitalWriteFast(THRUSTER_PINS[i], on ? HIGH : LOW);
  }
}

// =========================================================
// Puck output
// =========================================================
void setPuck(bool on) {

  pinMode(AIR_BEARING_PIN, OUTPUT);
  digitalWriteFast(AIR_BEARING_PIN, on ? HIGH : LOW);

  currentPuck = on;
}

// =========================================================
// Packet handler
// =========================================================
void processPacket() {

  float values[NUM_FLOATS];

  memcpy(values, rxBuffer, PAYLOAD_BYTES);

  // Refresh watchdog
  lastPacketTime = millis();
  simulinkActive = true;

  // Thrusters
  for (int i = 0; i < NUM_THRUSTERS; i++) {
    setThrusterDuty(i, values[i]);
  }

  // Puck
  setPuck(values[8] >= 0.5f);
}

// =========================================================
// Setup
// =========================================================
void setup() {

  for (int i = 0; i < NUM_THRUSTERS; i++) {
    pinMode(THRUSTER_PINS[i], OUTPUT);
    digitalWriteFast(THRUSTER_PINS[i], LOW);
  }

  pinMode(AIR_BEARING_PIN, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);

  // Safe startup state
  for (int i = 0; i < NUM_THRUSTERS; i++) {
    setThrusterDuty(i, 0.0f);
  }

  setPuck(false);

  digitalWrite(STATUS_LED_PIN, LOW);

  // Anchor PWM cycle to boot time
  pwmCycleStart_ms = millis();

  Serial.begin(115200);
}

// =========================================================
// Main loop
// =========================================================
void loop() {

  // =======================================================
  // Read serial bytes
  // =======================================================
  while (Serial.available() > 0) {

    uint8_t b = Serial.read();

    switch (rxState) {

      case WAITING_SYNC:

        if (b == SYNC_BYTE) {

          rxIndex = 0;
          rxState = READING_PAYLOAD;
        }

        break;
 
      case READING_PAYLOAD:

        rxBuffer[rxIndex++] = b;

        if (rxIndex >= PAYLOAD_BYTES) {

          processPacket();

          rxState = WAITING_SYNC;
        }

        break;
    }
  }

  // =======================================================
  // Watchdog timeout
  // =======================================================
  if (simulinkActive &&
      (millis() - lastPacketTime > PACKET_TIMEOUT_MS)) {

    for (int i = 0; i < NUM_THRUSTERS; i++) {
      setThrusterDuty(i, 0.0f);
    }

    //setPuck(false);

    simulinkActive = false;
  }

  // =======================================================
  // PWM tick
  // =======================================================
  updateSoftwarePWM();

  // =======================================================
  // Status LED
  // =======================================================
  digitalWrite(STATUS_LED_PIN,
               simulinkActive ? HIGH : LOW);
}