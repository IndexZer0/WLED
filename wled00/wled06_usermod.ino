/*
 * This file allows you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * EEPROM bytes 2750+ are reserved for your custom use case. (if you extend #define EEPSIZE in wled01_eeprom.h)
 * bytes 2400+ are currently ununsed, but might be used for future wled features
 */

//Use userVar0 and userVar1 (API calls &U0=,&U1=, uint16_t)

/*
 * For NodeMCU 2866.
 */

#include <Wire.h>

#define TOP_STAIRS_SENSOR 1
#define BOTTOM_STAIRS_SENSOR 2
int triggeredOnSensor = NULL;

#define TOP_STAIRS_XSHUT_PIN 12    // GPIO12 = NodeMCU D6 
#define BOTTOM_STAIRS_XSHUT_PIN 13 // GPIO13 = NodeMCU D7

#define TOP_STAIRS_ADDRESS 41    // 41 = 0x29
#define BOTTOM_STAIRS_ADDRESS 48 // 48 = 0x30

#define TOP_STAIRS_THRESHOLD 600
#define BOTTOM_STAIRS_THRESHOLD 1000

VL53L0X_Sensor topStairsSensor = VL53L0X_Sensor(TOP_STAIRS_XSHUT_PIN, (uint8_t) TOP_STAIRS_ADDRESS, "Top Stairs", TOP_STAIRS_THRESHOLD);
VL53L0X_Sensor bottomStairsSensor = VL53L0X_Sensor(BOTTOM_STAIRS_XSHUT_PIN, (uint8_t) BOTTOM_STAIRS_ADDRESS, "Bottom Stairs", BOTTOM_STAIRS_THRESHOLD);

#define EFFECT_SPEED 255
#define EFFECT_INTENSITY 160

#define STATE_OFF 0
#define STATE_FLOW_TRANSITION_ON 1
#define STATE_FLOWING 2
#define STATE_FLOW_TRANSITION_OFF 3
int currentState = STATE_OFF;
long stateChangedAt = NULL;

#define MAX_FLOW_DURATION 20000
#define FLOW_TRANSITION_ON_DURATION 4000
#define FLOW_TRANSITION_OFF_DURATION 4000

#define LOOP_DELAY 200
long lastLoopTime = 0;

//gets called once at boot. Do all initialization that doesn't depend on network here
void userSetup()
{
  setupSensors();
  stateOff();
}

void setupSensors()
{
  Wire.begin();
  
  // reset both sensors.
  topStairsSensor.shutdown();
  bottomStairsSensor.shutdown();
  delay(10);
  topStairsSensor.boot();
  bottomStairsSensor.boot();
  delay(10);

  topStairsSensor.shutdown();
  bottomStairsSensor.setAddress();
  topStairsSensor.boot();

  topStairsSensor.init();
  bottomStairsSensor.init();

  logAddresses();
}

void logAddresses()
{
  topStairsSensor.logAddress();
  bottomStairsSensor.logAddress();
}

//gets called every time WiFi is (re-)connected. Initialize own network interfaces here
void userConnected()
{
  
}

//loop. You can use "if (WLED_CONNECTED)" to check for successful connection
void userLoop()
{
  int currentTime = millis();

  if (currentState == STATE_FLOW_TRANSITION_OFF) {
    if (currentTime - stateChangedAt > FLOW_TRANSITION_OFF_DURATION) {
      stateOff();
    }
  }

  if (currentState == STATE_FLOW_TRANSITION_ON) {
    if (currentTime - stateChangedAt > FLOW_TRANSITION_ON_DURATION) {
      stateFlowing();
    }
  }

  if (currentState == STATE_FLOWING) {
    if (currentTime - stateChangedAt > MAX_FLOW_DURATION) {
      stateFlowTransitionOff();
      return;
    }
  }

  if (currentTime - lastLoopTime < LOOP_DELAY)
  {
    return;
  }

  lastLoopTime = currentTime;

  if (topStairsSensor.wasTriggered()) {
    if (currentState == STATE_OFF) {

      triggeredOnSensor = TOP_STAIRS_SENSOR;
      stateFlowTransitionOn();

    } else if (currentState == STATE_FLOWING && triggeredOnSensor == BOTTOM_STAIRS_SENSOR) {

      stateFlowTransitionOff();
      
    }
  }

  if (bottomStairsSensor.wasTriggered()) {
    if (currentState == STATE_OFF) {

      triggeredOnSensor = BOTTOM_STAIRS_SENSOR;
      stateFlowTransitionOn();

    } else if (currentState == STATE_FLOWING && triggeredOnSensor == TOP_STAIRS_SENSOR) {

      stateFlowTransitionOff();
      
    }
  }

}

void stateChanged() {
  stateChangedAt = millis();
  strip.effectStartedAt = millis();
}

void stateFlowTransitionOn()
{
  Serial.println("[stateFlowTransitionOn]");

  currentState = STATE_FLOW_TRANSITION_ON;
  stateChanged();
  
  bri = 128;

  effectCurrent = FX_MODE_RAINBOW_CYCLE_TRANSITION_ON;
  effectSpeed = EFFECT_SPEED;
  effectIntensity = EFFECT_INTENSITY;
  
  resetTimebase();

  bool reverse = triggeredOnSensor == TOP_STAIRS_SENSOR;
  WS2812FX::Segment &seg = strip.getSegment(0);
  seg.setOption(1, reverse);

  // TODO?

  transitionDelayTemp = 0; // no transition
  colorUpdated(3);
}

void stateFlowing()
{
  Serial.println("[stateFlowing]");

  currentState = STATE_FLOWING;
  stateChanged();

  effectCurrent = FX_MODE_RAINBOW_CYCLE;
  effectSpeed = EFFECT_SPEED;
  effectIntensity = EFFECT_INTENSITY;

  // TODO?

  transitionDelayTemp = 0; // no transition
  colorUpdated(3);
}

void stateFlowTransitionOff()
{
  Serial.println("[stateFlowTransitionOff]");

  currentState = STATE_FLOW_TRANSITION_OFF;
  stateChanged();

  effectCurrent = FX_MODE_RAINBOW_CYCLE_TRANSITION_OFF;
  effectSpeed = EFFECT_SPEED;
  effectIntensity = EFFECT_INTENSITY;

  bool reverse = triggeredOnSensor == TOP_STAIRS_SENSOR;
  WS2812FX::Segment &seg = strip.getSegment(0);
  seg.setOption(1, reverse);

  // TODO?

  transitionDelayTemp = 0; // no transition
  colorUpdated(3);
}

void stateOff()
{
  Serial.println("[stateOff]");

  currentState = STATE_OFF;
  stateChanged();

  triggeredOnSensor = NULL;
  bri = 0;

  effectCurrent = FX_MODE_STATIC;

  // TODO?
  
  transitionDelayTemp = 0; // no transition
  colorUpdated(3);
}
