#ifndef WEMOS_D1_NTPPROVIDER_H_
#define WEMOS_D1_NTPPROVIDER_H_

#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <WiFiUdp.h>
#include <API.h>

#define NTP_SERVICE_NAME "_ntp_service_"

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

class Time : public ESP_Service {
protected:
    Ticker *time_update = NULL;
    WiFiUDP *socket = NULL;
    UTC_TIME_ZONES time_zone;
    unsigned long epoch;
    boolean ntp_update = false;

private:
    void send_ntp_packet(const char *);

    void recieve_ntp_packet();

    void static timer_tick(unsigned long *epoch) {
        (*epoch)++;
#ifdef __DEBUG__
        print_time(*epoch);
#endif
    }

public:
    Time(const uint16_t port, unsigned long time = 0) {
        time_update = new Ticker();
        time_update->attach(1, timer_tick, &epoch);
        set_time(time);
        socket = new WiFiUDP();
        socket->begin(port);
    }

    void set_time_zone(UTC_TIME_ZONES zone) { time_zone = zone; };

    UTC_TIME_ZONES get_time_zone() { return time_zone; }

    void set_time(unsigned long t) {
        time_update->detach();
        epoch = t + time_zone * 60;
        time_update->attach(1, timer_tick, &epoch);
    }

    unsigned long get_time() { return epoch; }

    void update_time_ntp() { ntp_update = true; }

#ifdef __DEBUG__

    void static print_time(unsigned long epoch);

#endif

    const char *get_name() { return NTP_SERVICE_NAME; };

    void cycle_routine();

    virtual ~Time() {
        delete time_update;
        delete socket;
    }
};

#endif /* WEMOS_D1_NTPPROVIDER_H_ */
