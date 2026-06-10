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
// PWM configuration (ISR-driven software PWM)
// =========================================================
const float    PWM_FREQUENCY        = 5.0f;
const uint32_t PWM_TICK_HZ          = 10000;                           // 10 kHz ISR
const uint32_t PWM_TICK_INTERVAL_US = 1000000UL / PWM_TICK_HZ;         // 100 us
const uint32_t PWM_TICKS_PER_PERIOD =
(uint32_t)(PWM_TICK_HZ / PWM_FREQUENCY);                           // 2000 ticks

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
volatile float    currentDuty[8] = {0};   // written from main, read from ISR
volatile uint32_t pwmTick        = 0;     // ISR only

bool currentPuck = false;

uint8_t rxBuffer[PAYLOAD_BYTES];
size_t  rxIndex = 0;

enum {
  WAITING_SYNC,
  READING_PAYLOAD
} rxState = WAITING_SYNC;

unsigned long lastPacketTime = 0;
bool          simulinkActive = false;

IntervalTimer pwmTimer;

// =========================================================
// PWM ISR — fires every PWM_TICK_INTERVAL_US
// =========================================================
void pwmISR() {

  uint32_t pos = pwmTick;

  if (++pwmTick >= PWM_TICKS_PER_PERIOD) {
    pwmTick = 0;
  }

  for (int i = 0; i < NUM_THRUSTERS; i++) {

    float pct = currentDuty[i];
    bool  on;

    if (pct <= 0.0f) {
      on = false;
    } else if (pct >= 100.0f) {
      on = true;
    } else {

      uint32_t onTicks =
      (uint32_t)((pct / 100.0f) * PWM_TICKS_PER_PERIOD);

      on = (pos < onTicks);
    }

    digitalWriteFast(THRUSTER_PINS[i], on ? HIGH : LOW);
  }
}

// =========================================================
// Thruster duty update (called from main context)
// =========================================================
void setThrusterDuty(int idx, float pct) {

  if (isnan(pct) || pct < 0.0f)
    pct = 0.0f;

  if (pct > 100.0f)
    pct = 100.0f;

  currentDuty[idx] = pct;   // atomic 32-bit store on Cortex-M
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

  // Pin init — drive thrusters LOW before the ISR ever fires
  for (int i = 0; i < NUM_THRUSTERS; i++) {
    pinMode(THRUSTER_PINS[i], OUTPUT);
    digitalWriteFast(THRUSTER_PINS[i], LOW);
  }

  pinMode(AIR_BEARING_PIN, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);

  // Safe startup duty
  for (int i = 0; i < NUM_THRUSTERS; i++) {
    setThrusterDuty(i, 0.0f);
  }

  setPuck(false);

  digitalWrite(STATUS_LED_PIN, LOW);

  // Start PWM ISR
  pwmTimer.begin(pwmISR, PWM_TICK_INTERVAL_US);
  pwmTimer.priority(128);   // default mid-priority; Serial preempts cleanly

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

    setPuck(false);

  simulinkActive = false;
    }

    // =======================================================
    // Status LED
    // =======================================================
    digitalWrite(STATUS_LED_PIN,
                 simulinkActive ? HIGH : LOW);
}
