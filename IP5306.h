#include "esphome.h"

class IP5306 : public PollingComponent, public Sensor {
 public:
  Sensor *battery_level = new Sensor();
  Sensor *fully_charged = new Sensor();
  
  #define IP5306_ADDR           0x75
  #define IP5306_REG_SYS_CTL0   0x00
  #define IP5306_REG_READ1      0x71
  #define IP5306_REG_READ3      0x78
  #define CHARGE_FULL_BIT       3
  
  // Batterylevel 
  int8_t IP5306BatteryLevel() {
    Wire.beginTransmission(IP5306_ADDR);
    Wire.write(IP5306_REG_READ3);
    if (Wire.endTransmission(false) == 0
     && Wire.requestFrom(IP5306_ADDR, 1)) {
      switch (Wire.read() & 0xF0) {
      case 0xE0: return 25;
      case 0xC0: return 50;
      case 0x80: return 75;
      case 0x00: return 100;
      default: return 0;
      }
    }
    return -1;
  }
  
  bool IP5306FullyCharged() {
    uint8_t data;
    Wire.beginTransmission(IP5306_ADDR);
    Wire.write(IP5306_REG_READ1);
    Wire.endTransmission(false);
    Wire.requestFrom(IP5306_ADDR, 1);
    data = Wire.read();
    if (data & (1 << CHARGE_FULL_BIT)) {
        return true;
    }
    else {
        return false;
    }
  }

/* 5th bit - Boost enable
   0x35 - 110101 - disable
   0x37 - 110111 - enable

   4th bit - Charger enable
   0x
*/
  // PowerBoostKeepOn
  void IP5306setPowerBoostKeepOn(bool en) {
    Wire.beginTransmission(IP5306_ADDR);
    Wire.write(IP5306_REG_SYS_CTL0);
    if (en) {
        // Set bit1: 1 enable 0 disable boost keep on
        Wire.write(0x37);
        ESP_LOGD("IP5306", "PowerBoostKeepOn ENABLED!");
    }
    else {
        // 0x35 is default reg value
        Wire.write(0x35);
        ESP_LOGD("IP5306", "PowerBoostKeepOn DISABLED!");
    }
    Wire.endTransmission();
  }

  // Update batterylevel every minute
  IP5306() : PollingComponent(60000) {}
  
  void setup() override {
    // Initialize the device here. Usually Wire.begin() will be called in here,
    // though that call is unnecessary if you have an 'i2c:' entry in your config

    Wire.begin();
    //delay(60000);
    IP5306setPowerBoostKeepOn(true);
  }
  
  void update() override {
    int batterylevel = IP5306BatteryLevel();
    battery_level->publish_state(batterylevel);
    bool charged = IP5306FullyCharged();
    fully_charged->publish_state(charged);
  }
};
