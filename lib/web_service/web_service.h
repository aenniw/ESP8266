#ifndef WEMOS_D1_WEBSERVICE_H_
#define WEMOS_D1_WEBSERVICE_H_

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <base64.h>
#include <API.h>
#include <FS.h>
#include <functional>

#define WEB_SERVICE_NAME    "_web_service_"
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

typedef std::function<String(String)> WebServiceFunction;

class WebService : public ESP_Service {
protected:
    ESP8266WebServer *web_server = NULL;
    DNSServer *dns_server = NULL;
    String *acc = NULL, *passwd = NULL;

private:
    void on_not_found();

    void on_invalid_credentials();

    bool valid_credentials();

    void tmp() {}

public:
    WebService() : WebService("", "") {}

    WebService(const char *, const char *);

    const char *get_name() { return WEB_SERVICE_NAME; };

    void add_handler(const char *, HTTPMethod, const char *, WebServiceFunction);

    void add_handler_auth(const char *, HTTPMethod, const char *, WebServiceFunction);

    void add_handler_file(const char *, HTTPMethod, const char *, const char *);

    void add_handler_file_auth(const char *, HTTPMethod, const char *, const char *);

    void cycle_routine();

    virtual ~WebService() {
        delete web_server;
        delete acc;
        delete passwd;
        //dns_server->stop();
        //delete dns_server;
    }
};

#endif /* WEMOS_D1_WEBSERVICE_H_ */
