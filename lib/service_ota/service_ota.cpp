#include "service_ota.h"

OtaService::OtaService(const char *password, uint16_t port) {
    ArduinoOTA.setPort(port);
    if (password != NULL) {
        ArduinoOTA.setPassword(password);
    }
    ArduinoOTA.onStart(on_start);
    ArduinoOTA.onEnd(on_end);
    ArduinoOTA.onProgress(on_progress);
    ArduinoOTA.onError(on_error);
    ArduinoOTA.begin();
}

OtaService *OtaService::get_instance(const char *password, uint16_t port) {
    static OtaService *otaService = new OtaService(password, port);
    return otaService;
}

void OtaService::on_start() {
    Log::println("OTA Start");
}

void OtaService::on_end() {
    Log::println("OTA End");
}

void OtaService::on_progress(unsigned int progress, unsigned int total) {
    Log::println("Progress: %u%%\r", (progress / (total / 100)));
}

void OtaService::on_error(ota_error_t error) {
    Log::println("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
        Log::println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR)
        Log::println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR)
        Log::println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR)
        Log::println("Receive Failed");
    else if (error == OTA_END_ERROR)
        Log::println("End Failed");
}