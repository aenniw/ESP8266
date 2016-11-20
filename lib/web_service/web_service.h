#ifndef WEMOS_D1_WEBSERVICE_H_
#define WEMOS_D1_WEBSERVICE_H_

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <base64.h>

#define DNS_HOSTNAME "weemos"
#define DNS_HOSTNAME_AP "weemos.local"

#define RESP_JSON "application/json"
#define RESP_HTML "text/html"
#define RESP_TEXT "text/plain"

typedef String (*WebServiceFunction)(String);

class WebService {
    ESP8266WebServer *web_server = NULL;
    DNSServer *dns_server = NULL;
    String credentials;

private:
    void on_not_found();

    bool valid_credentials();

    void tmp() {}

public:
    WebService() : WebService("", "") {}

    WebService(String user, String pass);

    void add_handler(const char *, HTTPMethod, const char *, WebServiceFunction);

    void add_handler_auth(const char *, HTTPMethod, const char *,
                          WebServiceFunction);

    void cycle_routine();

    virtual ~WebService() {
        delete web_server;
        dns_server->stop();
        delete dns_server;
    }
};

#endif /* WEMOS_D1_WEBSERVICE_H_ */
