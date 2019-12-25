#include "service_ntp.h"

NtpService::NtpService(const char *server, const int offset, const int refresh) {
    ntpUDP = new WiFiUDP();
    timeClient = new NTPClient(*ntpUDP, server, offset, refresh);
    timeClient->begin();
}

NtpService *NtpService::getInstance(const char *server, const int offset, const int refresh) {
    static auto *singleton = new NtpService(server, offset, refresh);
    return singleton;
}

void NtpService::cycle_routine() {
    timeClient->update();
}

unsigned long NtpService::get_time() const {
    return timeClient->getEpochTime();
}

NtpService::~NtpService() {
    delete timeClient;
    delete ntpUDP;
}