#include <ntp_provider.h>

const int NTP_PACKET_SIZE = 48;
byte packetBuffer[NTP_PACKET_SIZE]; // buffer to hold incoming and outgoing
                                    // packets
WiFiUDP udp_ntp;
Ticker time_update;
unsigned long epoch = 0;

void sendNTPpacket() {
  IPAddress timeServerIP; // time.nist.gov NTP server address
  WiFi.hostByName("time.nist.gov", timeServerIP);
  Serial.println("sending NTP packet...");
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011; // LI, Version, Mode
  packetBuffer[1] = 0;          // Stratum, or type of clock
  packetBuffer[2] = 6;          // Polling Interval
  packetBuffer[3] = 0xEC;       // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  udp_ntp.beginPacket(timeServerIP, 123); // NTP requests are to port 123
  udp_ntp.write(packetBuffer, NTP_PACKET_SIZE);
  udp_ntp.endPacket();
}

bool recieveNTPpacket() {
  int cb = udp_ntp.parsePacket();
  if (cb) {
    udp_ntp.read(packetBuffer, NTP_PACKET_SIZE);
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    epoch = secsSince1900 - 2208988800UL;
    return true;
  }
  return false;
}

void printTime() {
  Serial.print("The UTC time is ");
  // print the hour (86400 equals secs per day)
  Serial.print((epoch % 86400L) / 3600);
  Serial.print(':');
  // In the first 10 minutes of each hour, we'll want a leading '0'
  if (((epoch % 3600) / 60) < 10) {
    Serial.print('0');
  }
  // print the minute (3600 equals secs per minute)
  Serial.print((epoch % 3600) / 60);
  Serial.print(':');
  // In the first 10 seconds of each minute, we'll want a leading '0'
  if ((epoch % 60) < 10) {
    Serial.print('0');
  }
  // print the second
  Serial.println(epoch % 60);
}

void update_npt() {
  if (epoch == 0 && WiFi.isConnected())
    sendNTPpacket();
}

void setup_NTP(unsigned int localPort) {
  Serial.println("Starting UDP");
  udp_ntp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp_ntp.localPort());
  time_update.attach(1, []() {
    if (epoch == 0 && !recieveNTPpacket()) {
      return;
    }
    epoch++;
    printTime();
  });
}
