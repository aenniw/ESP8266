#include <ntp_provider.h>

void Time::send_ntp_packet(const char *ntp_server = "time.nist.gov") {
    IPAddress timeServerIP; // time.nist.gov NTP server address
    WiFi.hostByName(ntp_server, timeServerIP);
#ifdef __DEBUG__
    Serial.println("sending NTP packet...");
#endif
    socket->beginPacket(timeServerIP, 123); // NTP requests are to port 123
    socket->write((const byte[]) {
            0b11100011, 0, 6, 0xEC, 0, 0, 0, 0, 0, 0, 0, 0, 49, 0x4E, 49, 52,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 48);
    socket->endPacket();
}

void Time::recieve_ntp_packet() {
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

#ifdef __DEBUG__
void Time::print_time(unsigned long epoch) {
  Serial.print("The UTC time is ");
  Serial.print((epoch % 86400L) / 3600);
  Serial.print(':');
  if (((epoch % 3600) / 60) < 10) {
    Serial.print('0');
  }
  Serial.print((epoch % 3600) / 60);
  Serial.print(':');
  if ((epoch % 60) < 10) {
    Serial.print('0');
  }
  Serial.println(epoch % 60);
}
#endif

void Time::cycle_routine() {
    if (WiFi.status() == WL_CONNECTED && ntp_update) {
        if (epoch % 2)
            send_ntp_packet();
        recieve_ntp_packet();
    }
}
