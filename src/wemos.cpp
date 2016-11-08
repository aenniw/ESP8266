#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <aREST.h>

#include <file_system.h>
#include <led_strip.h>
#include <ntp_provider.h>
#include <ota.h>
#include <web_service.h>

char *SSID_local = NULL, *PASSWD_local = NULL, *SSID_remote = NULL,
     *PASSWD_remote = NULL;

aREST rest = aREST();

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
  case WIFI_EVENT_STAMODE_GOT_IP:
    Serial.printf("WiFi connected IP address: %s\n",
                  WiFi.localIP().toString().c_str());
    break;
  case WIFI_EVENT_STAMODE_DISCONNECTED:
    Serial.println("WiFi lost connection");
    break;
  }
}

void setup() {
  Serial.begin(115200);
  if (!SPIFFS.begin() || !load_config())
    Serial.println("Failed to load config");
  WiFi.onEvent(WiFiEvent);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(SSID_local, PASSWD_local);
  WiFi.begin(SSID_remote, PASSWD_remote);
  Serial.printf("AP IP address: %s\n", WiFi.softAPIP().toString().c_str());

  setup_strip(10);
  setup_NTP(7777);
  setup_web_server(SSID_local);
  setup_OTA(8266, SSID_local, PASSWD_local);
  strip_mode(RAINBOW_CYCLE, 50);
}

void loop() {
  update_npt();
  handle_web_server_client();
}
