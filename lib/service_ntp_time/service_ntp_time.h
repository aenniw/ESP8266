#ifndef NTP_TIME_SERVICE_H_
#define NTP_TIME_SERVICE_H_

#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <WiFiUdp.h>
#include <commons.h>
#include <logger.h>

typedef enum {
    UTC__12_00 = -720,
    UTC__11_00 = -660,
    UTC__10_00 = -600,
    UTC__09_30 = -570,
    UTC__09_00 = -540,
    UTC__08_00 = -480,
    UTC__07_00 = -420,
    UTC__06_00 = -360,
    UTC__05_00 = -300,
    UTC__04_00 = -240,
    UTC__03_30 = -210,
    UTC__03_00 = -180,
    UTC__02_00 = -120,
    UTC__01_00 = -60,
    UTC__00_00 = 0,
    UTC_01_00 = 60,
    UTC_02_00 = 120,
    UTC_03_00 = 180,
    UTC_03_30 = 210,
    UTC_04_00 = 240,
    UTC_04_30 = 270,
    UTC_05_00 = 300,
    UTC_05_30 = 330,
    UTC_05_45 = 345,
    UTC_06_00 = 360,
    UTC_06_30 = 390,
    UTC_07_00 = 420,
    UTC_08_00 = 480,
    UTC_08_30 = 510,
    UTC_08_45 = 525,
    UTC_09_00 = 540,
    UTC_09_30 = 570,
    UTC_10_00 = 600,
    UTC_10_30 = 630,
    UTC_11_00 = 660,
    UTC_12_00 = 720,
    UTC_12_45 = 765,
    UTC_13_00 = 780,
    UTC_14_00 = 840
} UTC_TIME_ZONES;

class NtpTimeService : public ESP_Service {
protected:
    Ticker *time_update = NULL;
    WiFiUDP *socket = NULL;
    UTC_TIME_ZONES time_zone;
    unsigned long epoch;
    boolean ntp_update = false;

private:
    void send_ntp_packet(const char *);

    void recieve_ntp_packet();

    void static timer_tick(unsigned long *epoch) { (*epoch)++; }

    NtpTimeService(const uint16_t port, unsigned long time = 0);

public:
    static NtpTimeService *get_instance(const uint16_t port, const UTC_TIME_ZONES zone = UTC_01_00);

    void set_time_zone(UTC_TIME_ZONES zone) { time_zone = zone; };

    UTC_TIME_ZONES get_time_zone() { return time_zone; }

    void set_time(unsigned long t);

    unsigned long get_time() { return epoch; }

    void update_time_ntp() { ntp_update = true; }

    void cycle_routine();

    virtual ~NtpTimeService();
};

#endif /* NTP_TIME_SERVICE_H_ */
