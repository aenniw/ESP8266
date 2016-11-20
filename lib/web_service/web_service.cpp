#include <web_service.h>

void WiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case WIFI_EVENT_STAMODE_GOT_IP:
            if (MDNS.begin(DNS_HOSTNAME))
                MDNS.addService("http", "tcp", 80);
#ifdef __DEBUG__
        else
          Serial.println("Error setting up MDNS responder!");
#endif
            break;
        case WIFI_EVENT_STAMODE_DISCONNECTED:
#ifdef __DEBUG__
            Serial.println("WiFi lost connection");
#endif
            break;
        case WIFI_EVENT_STAMODE_CONNECTED:
#ifdef __DEBUG__
            Serial.println("WiFi accepted");
#endif
            break;
        case WIFI_EVENT_STAMODE_AUTHMODE_CHANGE:
            break;
        case WIFI_EVENT_STAMODE_DHCP_TIMEOUT:
            break;
        case WIFI_EVENT_SOFTAPMODE_STACONNECTED:
#ifdef __DEBUG__
            Serial.println("Client connected");
#endif
            break;
        case WIFI_EVENT_SOFTAPMODE_STADISCONNECTED:
#ifdef __DEBUG__
            Serial.println("Client disconnected");
#endif
            break;
        case WIFI_EVENT_SOFTAPMODE_PROBEREQRECVED:
            break;
        case WIFI_EVENT_MAX:
            break;
        case WIFI_EVENT_MODE_CHANGE:
            break;
    }
}

WebService::WebService(String user, String pass) {
    WiFi.onEvent(WiFiEvent);
    WiFi.mode(WIFI_AP_STA);

    dns_server = new DNSServer();
    dns_server->setErrorReplyCode(DNSReplyCode::NoError);
    dns_server->start(53, DNS_HOSTNAME_AP, IPAddress(192, 168, 4, 1));

    if (user.length() != 0 && pass.length() != 0) {
        credentials = base64::encode(user + ":" + pass);
#ifdef __DEBUG__
        Serial.println(user + ":" + pass);
        Serial.println("Credentials " + credentials);
#endif
    }
    web_server = new ESP8266WebServer(80);
    web_server->onNotFound([this]() { on_not_found(); });
    web_server->begin();
}

void WebService::on_not_found() {
#ifdef __DEBUG__
    String message = "Not Found\n\nURI: ";
    message += web_server->uri();
    message += "\nMethod: ";
    message += (web_server->method() == HTTP_GET) ? "GET" : "POST";
    message += "\nHeaders: ";
    message += web_server->headers();
    message += "\n";
    for (uint8_t i = 0; i < web_server->headers(); i++) {
      message +=
          " " + web_server->headerName(i) + ": " + web_server->header(i) + "\n";
    }
    message += "\nArguments: " + web_server->args();
    message += "\n";
    for (uint8_t i = 0; i < web_server->args(); i++) {
      message += " " + web_server->argName(i) + ": " + web_server->arg(i) + "\n";
    }
    web_server->send(404, RESP_TEXT, message);
#else
    web_server->send(404, RESP_TEXT, "Not found");
#endif
}

bool WebService::valid_credentials() {
    if (credentials.length() == 0)
        return true;
    for (uint8_t i = 0; i < web_server->headers(); i++) {
        if (web_server->headerName(i).compareTo("Authorization") == 0 &&
            web_server->header(i).compareTo("Basic " + credentials) == 0) {
#ifdef __DEBUG__
            Serial.println(web_server->header(i) + " == " + credentials + " ?");
#endif
            return true;
        }
    }
    return false;
}

void WebService::add_handler(const char *uri, HTTPMethod method,
                             const char *resp_type,
                             WebServiceFunction handler) {
    web_server->on(uri, [=]() {
#ifdef __DEBUG__
        Serial.printf("Recieved on %s\n", uri);
#endif
        if (method == HTTP_ANY || method == web_server->method()) {
#ifdef __DEBUG__
            String resp = handler(web_server->arg("plain"));
            web_server->send(200, resp_type, resp);
#else
            web_server->send(200, resp_type, handler(web_server->arg("plain")));
#endif
        } else
            on_not_found();
    });
}

void WebService::add_handler_auth(const char *uri, HTTPMethod method,
                                  const char *resp_type,
                                  WebServiceFunction handler) {
    web_server->on(uri, [=]() {
#ifdef __DEBUG__
        Serial.printf("Recieved on %s\n", uri);
#endif
        if ((valid_credentials()) &&
            (method == HTTP_ANY || method == web_server->method())) {
#ifdef __DEBUG__
            String resp = handler(web_server->arg("plain"));
            web_server->send(200, resp_type, resp);
#else
            web_server->send(200, resp_type, handler(web_server->arg("plain")));
#endif
        } else
            on_not_found();
    });
}

void WebService::cycle_routine() {
    dns_server->processNextRequest();
    web_server->handleClient();
}
