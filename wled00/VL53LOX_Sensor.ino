#include <VL53L0X.h>

class VL53L0X_Sensor
{
  private:
    VL53L0X sensor;
    int xshut_pin;
    uint8_t address;
    String name;
    int triggerThreshold;
    long lastTriggeredAt;

  public:
    VL53L0X_Sensor(int xshut_pin, uint8_t address, String name, int triggerThreshold) {
      this->xshut_pin = xshut_pin;
      this->address = address;
      this->name = name;
      this->triggerThreshold = triggerThreshold;

      pinMode(xshut_pin, OUTPUT);
    }

    void shutdown() {
      digitalWrite(xshut_pin, LOW);
    }

    void boot() {
      digitalWrite(xshut_pin, HIGH);
    }

    void setAddress() {
      sensor.setAddress(address);
    }

    void logAddress() {
      uint8_t address = sensor.getAddress();
      Serial.print("[");
      Serial.print(name);
      Serial.print("] - Address ");
      Serial.print(address);
      Serial.print(" (0x");
      Serial.print(address, HEX);
      Serial.println(")\n");
    }

    void init() {
      if (!sensor.init())
      {
        Serial.print("[");
        Serial.print(name);
        Serial.println("] - Failed to detect and initialize");
        while (1)
        {
        }
      }

      sensor.setTimeout(500);

      // Start continuous back-to-back mode (take readings as
      // fast as possible).  To use continuous timed mode
      // instead, provide a desired inter-measurement period in
      // ms (e.g. sensor.startContinuous(100)).

      sensor.startContinuous(200);
    }

    bool wasTriggered() {
      int range = sensor.readRangeContinuousMillimeters();

      Serial.print("[");
      Serial.print(name);
      Serial.print("] - Range: ");
      Serial.print(range);

      if (range < triggerThreshold)
      {
        Serial.print(" [TRIGGERED]");
        lastTriggeredAt = millis();
        Serial.println("\n");
        return true;
      }

      if (sensor.timeoutOccurred())
      {
        Serial.print(" [TIMEOUT]");
      }

      Serial.println("\n");

      return false;
    }

};
