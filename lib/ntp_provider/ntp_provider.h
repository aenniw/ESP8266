#ifndef NTPPROVIDER_H_
#define NTPPROVIDER_H_

#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <WiFiUdp.h>

void setup_NTP(unsigned int localPort);

void update_npt();
#endif /* NTPPROVIDER_H_ */
