#ifndef ESP8266_PROJECTS_ROOT_SERVICE_REST_ROBUST_H
#define ESP8266_PROJECTS_ROOT_SERVICE_REST_ROBUST_H

#include <file_system.h>
#include <commons_json.h>
#include <service_rest.h>
#include <ESP8266mDNS.h>

#define SESSION_ID "SESSION_ID"
#define HTML_LOGIN  "/login.html"
#define HTML_INDEX  "/index.html"
#define JS_BUNDLE   "/index.js"
#define JS_SW       "/sw.js"

typedef enum {
    ALL = 0xFFFFFFFF,
    CALLBACKS_WIFI = 0x1,
    HTML_ALL_FILES = 0x2,
    CALLBACKS_SYSTEM = 0x4
} REST_INIT;

class RestServiceRobust : public RestService {
private:
    uint32_t login_id = 0;
protected:
    uint32_t generate_login_id();

    void on_invalid_credentials() override;

    bool valid_credentials() override;
public:
    RestServiceRobust(const char *usr, const char *pass, const uint16_t p, const REST_INIT scope);
};

#endif //ESP8266_PROJECTS_ROOT_SERVICE_REST_ROBUST_H
