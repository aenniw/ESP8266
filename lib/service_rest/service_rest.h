#ifndef REST_SERVICE_H_
#define REST_SERVICE_H_

#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <file_system.h>
#include <esp_api.h>
#include <logger.h>

extern "C" {
#include <user_interface.h>
}

#define JSON_RESP_OK "{ \"result\" : true }"
#define JSON_RESP_NOK "{ \"result\" : false }"

#define HTML_INDEX "/index.html"
#define HTML_ADMINISTRATION "/html/administration.html"
#define HTML_STATUS "/html/status.html"
#define HTML_WIFI "/html/wifi.html"
#define HTML_LOG "/html/log.html"
#define CSS_COMMON "/css/common_style.css"
#define CSS_INDEX "/css/index_style.css"
#define CSS_LOGIN "/css/login_style.css"
#define JS_COMMON "/js/common.js"
#define JS_LOG "/js/log.js"
#define JS_ADMINISTRATION "/js/administration.js"
#define JS_STATUS "/js/status.js"

#define CONFIG_GLOBAL_JSON "/json/config-global.json"

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
    ALL, HTML, CALLBACKS_WIFI, CALLBACKS_SYSTEM, LOGGING
} REST_INIT;

class RestService : public ESP_Service {
protected:
    ESP8266WebServer *web_server = NULL;
    char *acc = NULL, *passwd = NULL;

private:
    void on_not_found();

    void on_invalid_credentials();

    bool valid_credentials();

    uint32_t get_login_id();

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
        if (acc != NULL)
            free(acc);
        if (passwd != NULL)
            free(passwd);
    }
};

#endif /* REST_SERVICE_H_ */
