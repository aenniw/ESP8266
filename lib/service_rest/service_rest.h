#ifndef REST_SERVICE_H_
#define REST_SERVICE_H_

// FIXME: use DNS for AP/AP-STA #include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <base64.h>
#include <esp_api.h>
#include <FS.h>
#include <functional>
#include <logger.h>

#define DNS_HOSTNAME        "weemos"
#define DNS_HOSTNAME_AP     "weemos.local"

#define JSON_RESP_OK        "{ \"result\" : true }"
#define JSON_RESP_NOK       "{ \"result\" : false }"

#define RESP_JS             "application/javascript"
#define RESP_GZ             "application/x-gzip"
#define RESP_ZIP            "application/x-zip"
#define RESP_PDF            "application/x-pdf"
#define RESP_JSON           "application/json"
#define RESP_XML            "application/xml"
#define RESP_ICO            "image/x-icon"
#define RESP_TEXT           "text/plain"
#define RESP_HTML           "text/html"
#define RESP_CSS            "text/css"

typedef std::function<String(String)> RestServiceFunction;

class RestService : public ESP_Service {
protected:
    ESP8266WebServer *web_server = NULL;
    String *acc = NULL, *passwd = NULL;

private:
    void on_not_found();

    void on_invalid_credentials();

    bool valid_credentials();

    void tmp() {}

public:
    RestService() : RestService("", "", 80) {}

    RestService(const char *, const char *, const uint16_t);

    void add_handler(const char *, HTTPMethod, const char *, RestServiceFunction);

    void add_handler_auth(const char *, HTTPMethod, const char *, RestServiceFunction);

    void add_handler_file(const char *, HTTPMethod, const char *, const char *);

    void add_handler_file_auth(const char *, HTTPMethod, const char *, const char *);

    void cycle_routine();

    virtual ~RestService() {
        delete web_server;
        delete acc;
        delete passwd;
    }
};

#endif /* REST_SERVICE_H_ */
