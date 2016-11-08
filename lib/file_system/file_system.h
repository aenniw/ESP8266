#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_
#include <ArduinoJson.h>
#include <FS.h>

bool load_config();
bool save_config();
String get_file_content(const char *file_name);
#endif /* FILESYSTEM_H_ */
