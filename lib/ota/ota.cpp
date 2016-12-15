#include <ota.h>

FAILSAFE_Service::FAILSAFE_Service(const char *password, const char *hostname, uint16_t port) {
    ArduinoOTA.setPort(port);
    if (hostname != NULL)
        ArduinoOTA.setHostname(hostname);
    if (password != NULL)
        ArduinoOTA.setPassword(password);
    ArduinoOTA.onStart(on_start);
    ArduinoOTA.onEnd(on_end);
    ArduinoOTA.onProgress(on_progress);
    ArduinoOTA.onError(on_error);
    ArduinoOTA.begin();
}

void FAILSAFE_Service::on_error(ota_error_t error) {
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
}