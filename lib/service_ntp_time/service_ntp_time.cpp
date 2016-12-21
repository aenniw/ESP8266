#include "service_ntp_time.h"

NtpTimeService::NtpTimeService(const uint16_t port, unsigned long time) {
    time_update = new Ticker();
    time_update->attach(1, timer_tick, &epoch);
    set_time(time);
    socket = new WiFiUDP();
    socket->begin(port);
}

NtpTimeService *NtpTimeService::get_instance(const uint16_t port, const UTC_TIME_ZONES zone) {
    static NtpTimeService *ntp_service = new NtpTimeService(port);
    ntp_service->set_time_zone(zone);
    ntp_service->update_time_ntp();
    return ntp_service;
}

void NtpTimeService::set_time(unsigned long t) {
    time_update->detach();
    epoch = t + time_zone * 60;
    time_update->attach(1, timer_tick, &epoch);
}


void NtpTimeService::send_ntp_packet(const char *ntp_server = "time.nist.gov") {
    IPAddress timeServerIP; // time.nist.gov NTP server address
    WiFi.hostByName(ntp_server, timeServerIP);
    Log::println("sending NTP packet...");
    socket->beginPacket(timeServerIP, 123); // NTP requests are to port 123
    socket->write((const byte[]) {
            0b11100011, 0, 6, 0xEC, 0, 0, 0, 0, 0, 0, 0, 0, 49, 0x4E, 49, 52,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 48);
    socket->endPacket();
}

void NtpTimeService::recieve_ntp_packet() {
    if (socket->parsePacket()) {
        time_update->detach();
        byte packet[4];
        // 40,41,42,43 packets are necessary -> 0,1,2,3
        for (uint8_t i = 0; i <= 40; i += 4)
            socket->read(packet, 4);
        // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
        set_time((word(packet[0], packet[1]) << 16 | word(packet[2], packet[3])) -
                 2208988800UL);
        ntp_update = false;
    }
}

void NtpTimeService::cycle_routine() {
    if (WiFi.status() == WL_CONNECTED && ntp_update) {
        if (epoch % 2)
            send_ntp_packet();
        recieve_ntp_packet();
    }
}


NtpTimeService::~NtpTimeService() {
    delete time_update;
    delete socket;
}
