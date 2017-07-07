#ifndef REST_SERVICE_H_
#define REST_SERVICE_H_

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <file_system.h>
#include <commons.h>
#include <devices.h>
#include <service_log.h>

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

#define HTML_LOGIN              "/login.html"

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
    RestService(const char *, const char *, const uint16_t);

    void add_handler(const char *, HTTPMethod, const char *, RestServiceFunction,
                     const bool authentication = 0);

    void add_handler_file(const char *, HTTPMethod, const char *, const char *,
                          const bool authentication = 0);

    void cycle_routine();

    virtual ~RestService() {
        delete web_server;
        checked_free(acc);
        checked_free(passwd);
    }
};

#endif /* REST_SERVICE_H_ */
