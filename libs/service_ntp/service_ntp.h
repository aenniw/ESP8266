#ifndef ESP8266_PROJECTS_ROOT_SERVICE_NTP_H
#define ESP8266_PROJECTS_ROOT_SERVICE_NTP_H

#include <NTPClient.h>
#include <WiFiUdp.h>
#include <commons.h>

class NtpService : public Service {
private:
    WiFiUDP *ntpUDP = nullptr;
    NTPClient *timeClient = nullptr;
protected:
    NtpService(const char *server, const int offset, const int refresh);

public:
    static NtpService *
    getInstance(const char *server = "europe.pool.ntp.org", const int offset = 3600, const int refresh = 60000);

    unsigned long get_time() const;

    void cycle_routine() override;

    ~NtpService();
};

#endif //ESP8266_PROJECTS_ROOT_SERVICE_NTP_H
