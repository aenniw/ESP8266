#ifndef WEMOS_D1_OTA_H_
#define WEMOS_D1_OTA_H_

#include <ArduinoOTA.h>
#include <Ticker.h>

void setup_OTA(const uint16_t port, const char *host_name,
               const char *password);

void stop_OTA();

void listen_for_OTA();

#endif /* WEMOS_D1_OTA_H_ */
