#include <file_system.h>

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