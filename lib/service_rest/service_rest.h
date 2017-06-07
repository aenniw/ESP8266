#ifndef REST_SERVICE_H_
#define REST_SERVICE_H_

#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <file_system.h>
#include <commons.h>
#include <devices.h>
#include <logger.h>

extern "C" {
#include <user_interface.h>
}

#define JSON_RESP_OK "{ \"result\" : true }"
#define JSON_RESP_NOK "{ \"result\" : false }"

#define RESP_JS "application/javascript"
#define RESP_GZ "application/x-gzip"
#define RESP_ZIP "application/x-zip"
#define RESP_PDF "application/x-pdf"
#define RESP_JSON "application/json"
#define RESP_XML "application/xml"
#define RESP_ICO "image/x-icon"
#define RESP_TEXT "text/plain"
#define RESP_HTML "text/html"
#define RESP_CSS "text/css"

typedef std::function<String(String)> RestServiceFunction;
typedef enum {
    ALL = 0xFFFFFFFF,
    CALLBACKS_WIFI = 0x1,
    CALLBACKS_SYSTEM = 0x2,
    LOGGING = 0x4,
    HTML_DIGITAL_IO = 0x8,
    HTML_COMMON_FILES = 0x10,
    HTML_ADMIN_FILES = 0x20,
    HTML_STATUS_FILES = 0x40,
    // 0x80 available
    HTML_ALL_FILES = 0xF8
} REST_INIT;

class RestService : public Service {
protected:
    ESP8266WebServer *web_server = NULL;
    char *acc = NULL, *passwd = NULL;
    uint32_t login_id = 0;

private:
    void on_not_found();

    void on_invalid_credentials();

    bool valid_credentials();

    uint32_t generate_login_id();

public:
    RestService() : RestService(NULL, NULL, 80) {}

    RestService(const char *, const char *, const uint16_t);

    void add_handler(const char *, HTTPMethod, const char *, RestServiceFunction,
                     const bool authentication = 0);

    void add_handler_file(const char *, HTTPMethod, const char *, const char *,
                          const bool authentication = 0);

    void cycle_routine();

    static RestService *initialize(RestService *, REST_INIT);

    virtual ~RestService() {
        delete web_server;
        checked_free(acc);
        checked_free(passwd);
    }
};

#endif /* REST_SERVICE_H_ */
