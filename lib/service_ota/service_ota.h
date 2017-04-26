#ifndef OTA_SERVICE_H_
#define OTA_SERVICE_H_

#include <ArduinoOTA.h>
#include <commons.h>
#include <logger.h>

class OtaService : public Service {
private:
    static void on_progress(unsigned int progress, unsigned int total);

    static void on_error(ota_error_t error);

    static void on_start();

    static void on_end();

    OtaService(const char *password, uint16_t port);

public:

    static OtaService *get_instance(const char *password, uint16_t port = 8266);

    void cycle_routine() { ArduinoOTA.handle(); }
};


#endif /* OTA_SERVICE_H_ */
