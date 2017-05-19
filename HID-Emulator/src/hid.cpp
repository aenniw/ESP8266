#include <Arduino.h>
#include <Wire.h>

#define WIFI_SSID "****"
#define WIFI_PASS "****"
#define I2C_ADDRESS   0x48              // 7 bit I2C address

void ICACHE_FLASH_ATTR setup() {
    Serial.begin(115200);
    Wire.begin(D2, D1);
    delay(100);
    Serial.write("Ready\n");
}

void i2c_write(const char *data) {
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(data);
    if (Wire.endTransmission()) {
        Serial.write("I2C transmission error.\n");
        return;
    }
    // AT response
    /*Wire.requestFrom(I2C_ADDRESS, 1, 1);
    Serial.print("Resp: ");
    while (Wire.available()) {
        Serial.write((char) (48 + Wire.read()));
    }
    Serial.println();*/
}

void loop() {
    if (Serial.available()) {
        String data = Serial.readStringUntil('\n');
        i2c_write(data.c_str());
        Serial.println("Send");
    }
    yield(); // WATCHDOG/WIFI feed
}
