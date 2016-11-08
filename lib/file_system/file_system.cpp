#include <file_system.h>

const char *config_file = "/config.json";
extern char *SSID_local, *PASSWD_local, *SSID_remote, *PASSWD_remote;

String get_file_content(const char *file_name) {
  File file = SPIFFS.open(file_name, "r");
  if (!file) {
    Serial.println("Failed to open config file");
    return "";
  }
  String data = file.readString();
  file.close();
  return data;
}

bool load_config() {
  File configFile = SPIFFS.open(config_file, "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }
  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[configFile.size()]);
  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), configFile.size());
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject &json = jsonBuffer.parseObject(buf.get());
  if (!json.success()) {
    Serial.println("Failed to parse config file");
    return false;
  }

  SSID_remote = (char *)json["SSID_remote"].asString();
  PASSWD_remote = (char *)json["PASSWD_remote"].asString();
  SSID_local = (char *)json["SSID_local"].asString();
  PASSWD_local = (char *)json["PASSWD_local"].asString();

  Serial.printf("\n%s %s %s %s\n", SSID_local, PASSWD_local, SSID_remote,
                PASSWD_remote);
  configFile.close();
  return true;
}

bool save_config() {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject &json = jsonBuffer.createObject();

  json["SSID_remote"] = SSID_remote;
  json["PASSWD_remote"] = PASSWD_remote;
  json["SSID_local"] = SSID_local;
  json["PASSWD_local"] = PASSWD_local;

  File configFile = SPIFFS.open(config_file, "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }
  json.printTo(configFile);
  configFile.close();
  return true;
}
