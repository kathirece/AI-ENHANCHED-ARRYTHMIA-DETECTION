#include <Arduino.h>
#include <cstdio>
#include <Wire.h>
#include <U8g2lib.h>
#include <MAX30100_PulseOximeter.h>

// Install the complete Edge Impulse export under firmware/lib to enable inference.
#if __has_include(<arrhythmia_inferencing.h>)
#include <arrhythmia_inferencing.h>
#define HAS_EDGE_IMPULSE_MODEL 1
#else
#define HAS_EDGE_IMPULSE_MODEL 0
#endif

namespace {

constexpr uint8_t kSdaPin = 21;
constexpr uint8_t kSclPin = 22;
constexpr uint32_t kSerialBaud = 115200;
constexpr uint32_t kSampleIntervalMs = 100;
constexpr uint32_t kDisplayIntervalMs = 500;
constexpr uint32_t kSensorRetryIntervalMs = 3000;
constexpr float kMinimumBpm = 30.0F;
constexpr float kMaximumBpm = 220.0F;

#if HAS_EDGE_IMPULSE_MODEL
constexpr size_t kWindowSize = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
#else
constexpr size_t kWindowSize = 10;
#endif

static_assert(kWindowSize > 0, "The inference input window must not be empty.");

PulseOximeter pox;
U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);

float bpmWindow[kWindowSize] = {};
size_t sampleIndex = 0;
float currentBpm = 0.0F;
float modelConfidence = 0.0F;
char modelLabel[24] = "Not installed";
bool sensorReady = false;
volatile bool beatSeen = false;
uint32_t lastSampleMs = 0;
uint32_t lastDisplayMs = 0;
uint32_t lastSensorAttemptMs = 0;

void onBeatDetected() {
  beatSeen = true;
}

const char *rateCategory(float bpm) {
  if (bpm < kMinimumBpm || bpm > kMaximumBpm) {
    return "No reading";
  }
  if (bpm < 60.0F) {
    return "Bradycardia";
  }
  if (bpm <= 100.0F) {
    return "Normal";
  }
  return "Tachycardia";
}

void drawLine(uint8_t y, const char *text) {
  display.drawStr(0, y, text);
}

void renderDisplay() {
  char line[32];
  display.clearBuffer();
  display.setFont(u8g2_font_6x12_tf);

  if (!sensorReady) {
    drawLine(12, "MAX30100 not found");
    drawLine(28, "Check SDA/SCL/power");
    drawLine(44, "Retrying...");
    display.sendBuffer();
    return;
  }

  snprintf(line, sizeof(line), "BPM: %.1f", currentBpm);
  drawLine(12, line);
  snprintf(line, sizeof(line), "Rate: %s", rateCategory(currentBpm));
  drawLine(28, line);
  snprintf(line, sizeof(line), "Model: %s", modelLabel);
  drawLine(44, line);
#if HAS_EDGE_IMPULSE_MODEL
  snprintf(line, sizeof(line), "Confidence: %.1f%%", modelConfidence * 100.0F);
#else
  snprintf(line, sizeof(line), "Samples: %u/%u",
           static_cast<unsigned>(sampleIndex),
           static_cast<unsigned>(kWindowSize));
#endif
  drawLine(60, line);
  display.sendBuffer();
}

#if HAS_EDGE_IMPULSE_MODEL
int getSignalData(size_t offset, size_t length, float *outPtr) {
  if (offset + length > kWindowSize) {
    return -1;
  }
  for (size_t i = 0; i < length; ++i) {
    outPtr[i] = bpmWindow[offset + i];
  }
  return 0;
}

void runModel() {
  signal_t signal;
  signal.total_length = kWindowSize;
  signal.get_data = getSignalData;

  ei_impulse_result_t result = {};
  const EI_IMPULSE_ERROR error = run_classifier(&signal, &result, false);
  if (error != EI_IMPULSE_OK) {
    snprintf(modelLabel, sizeof(modelLabel), "Error %d", static_cast<int>(error));
    modelConfidence = 0.0F;
    Serial.printf("Inference failed: %d\n", static_cast<int>(error));
    return;
  }

  size_t bestIndex = 0;
  for (size_t i = 1; i < EI_CLASSIFIER_LABEL_COUNT; ++i) {
    if (result.classification[i].value > result.classification[bestIndex].value) {
      bestIndex = i;
    }
  }

  snprintf(modelLabel, sizeof(modelLabel), "%s",
           result.classification[bestIndex].label);
  modelConfidence = result.classification[bestIndex].value;
  Serial.printf("Model: %s | confidence: %.2f%%\n",
                modelLabel, modelConfidence * 100.0F);
}
#else
void runModel() {
  // The transparent range result remains available until a model is installed.
}
#endif

void addBpmSample(float bpm) {
  if (bpm < kMinimumBpm || bpm > kMaximumBpm) {
    return;
  }

  bpmWindow[sampleIndex++] = bpm;
  if (sampleIndex == kWindowSize) {
    runModel();
    sampleIndex = 0;
  }
}

bool startSensor() {
  if (!pox.begin()) {
    return false;
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
  pox.setOnBeatDetectedCallback(onBeatDetected);
  return true;
}

}  // namespace

void setup() {
  Serial.begin(kSerialBaud);
  Wire.begin(kSdaPin, kSclPin);
  display.begin();
  display.setContrast(180);

  Serial.println("ESP32 heart-rate classification prototype");
  Serial.println("Educational use only - not a medical diagnostic device.");
#if HAS_EDGE_IMPULSE_MODEL
  Serial.printf("Edge Impulse model found; input size: %u\n",
                static_cast<unsigned>(kWindowSize));
#else
  Serial.println("Edge Impulse model not installed; using BPM ranges only.");
#endif

  sensorReady = startSensor();
  lastSensorAttemptMs = millis();
  renderDisplay();
}

void loop() {
  const uint32_t now = millis();

  if (!sensorReady) {
    if (now - lastSensorAttemptMs >= kSensorRetryIntervalMs) {
      lastSensorAttemptMs = now;
      sensorReady = startSensor();
      Serial.println(sensorReady ? "MAX30100 initialized." : "MAX30100 retry failed.");
      renderDisplay();
    }
    return;
  }

  // The MAX30100 library must be serviced as frequently as possible.
  pox.update();

  if (now - lastSampleMs >= kSampleIntervalMs) {
    lastSampleMs = now;
    const float reading = pox.getHeartRate();
    if (reading >= kMinimumBpm && reading <= kMaximumBpm) {
      currentBpm = reading;
      addBpmSample(reading);
    }
  }

  if (beatSeen) {
    beatSeen = false;
    Serial.println("Beat detected");
  }

  if (now - lastDisplayMs >= kDisplayIntervalMs) {
    lastDisplayMs = now;
    Serial.printf("BPM: %.1f | rate category: %s\n",
                  currentBpm, rateCategory(currentBpm));
    renderDisplay();
  }
}
