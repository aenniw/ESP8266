#include <ota.h>

Ticker ota_timer;

void setup_OTA(const uint16_t port, const char *host_name,
               const char *password) {
  ArduinoOTA.setPort(port);
  // Hostname defaults to esp8266-[ChipID]
  if (host_name != NULL)
    ArduinoOTA.setHostname(host_name);
  // No authentication by default
  if (password != NULL)
    ArduinoOTA.setPassword(password);
  ArduinoOTA.onStart([]() { Serial.println("Start"); });
  ArduinoOTA.onEnd([]() { Serial.println("End"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
      Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR)
      Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR)
      Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR)
      Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR)
      Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}
void stop_OTA() { ota_timer.detach(); }

void listen_for_OTA() {
  stop_OTA();
  ota_timer.attach(1, []() { ArduinoOTA.handle(); });
}
