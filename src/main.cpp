#include <Homie.h>
#include <Bounce2.h>
#include <Arduino.h>
#include <stdint.h>

#include <Homie.h>

enum state {
  LOW_STATE = 0,
  HIGH_STATE,
  INITIALIZING,
};

const int SEND_INTERVAL = 60000;
const int POLL_INTERVAL = 100;
const int HYSTERESIS = 3;
const uint8_t THRESHOLD_HIGH = 60;
const uint8_t THRESHOLD_LOW = 30;

enum state current_state;
uint8_t lastPollValue = 0;
uint8_t maxValue = 0;
uint8_t minValue = 0xff;
uint32_t lastSendTime=0;
uint32_t lastPollTime=0;
uint32_t pulsecount =0;

HomieNode pulseNode("pulses", "Pulse Counter", "pulsecounter");

void loopHandler() {
  
  if(millis() - lastPollTime > POLL_INTERVAL){
    uint8_t v = analogRead(A0);
    enum state next_state = current_state;
    switch (current_state) {
    case INITIALIZING:
      if (v > THRESHOLD_HIGH) {
        next_state = HIGH_STATE;
      } else {
        next_state = LOW_STATE;
      }
      break;
    case HIGH:
      if (v < THRESHOLD_LOW) {
        next_state = LOW_STATE;
      }
      break;
    case LOW:
      if (v > THRESHOLD_HIGH) {
        next_state = HIGH_STATE;
        pulsecount++;
        pulseNode.setProperty("pulses").send(String(pulsecount));
      }
      break;
    }

    current_state = next_state;
    
    lastPollTime = millis();
    lastPollValue = v;
    maxValue = max(v, maxValue);
    minValue = min(v, minValue);
  }

  if (millis() - lastSendTime > SEND_INTERVAL ){
    pulseNode.setProperty("pulses").send(String(pulsecount));
    pulseNode.setProperty("min").send(String(minValue));
    pulseNode.setProperty("max").send(String(maxValue));
    pulseNode.setProperty("lastValue").send(String(lastPollValue));
    lastSendTime = millis();
  }
}

void setup() {
  current_state = INITIALIZING;
  lastSendTime=millis();
  Serial.begin(115200);
  Serial << endl << endl;
  Homie_setFirmware("water meter", "1.0.0");
  Homie.setLoopFunction(loopHandler);
  pulseNode.advertise("pulses").setName("pulses")
    .setDatatype("integer")
    .setRetained(false);
  pulseNode.advertise("max").setName("max measured value")
    .setDatatype("integer")
    .setRetained(false);
  pulseNode.advertise("min").setName("min measured value")
    .setDatatype("integer")
    .setRetained(false);
  pulseNode.advertise("lastValue").setName("last measured value")
    .setDatatype("integer")
    .setRetained(false);
  Homie.setup();
}

void loop() {
  Homie.loop();
}

