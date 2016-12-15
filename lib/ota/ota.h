#ifndef WEMOS_D1_OTA_H_
#define WEMOS_D1_OTA_H_

#include <ArduinoOTA.h>
#include <API.h>

#define FAILSAFE_SERVICE_NAME "_failsafe_service_"

class FAILSAFE_Service : public ESP_Service {
protected:
private:
    static void on_progress(unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    }

    static void on_error(ota_error_t error);

    static void on_start() { Serial.println("Start"); }

    static void on_end() { Serial.println("End"); }

public:
    FAILSAFE_Service() : FAILSAFE_Service(NULL, 8266) {}

    FAILSAFE_Service(const char *password, uint16_t port = 8266) : FAILSAFE_Service(password, NULL, port) {}

    FAILSAFE_Service(const char *password, const char *hostname, uint16_t port);

    const char *get_name() { return FAILSAFE_SERVICE_NAME; };

    void cycle_routine() { ArduinoOTA.handle(); }
};


#endif /* WEMOS_D1_OTA_H_ */
