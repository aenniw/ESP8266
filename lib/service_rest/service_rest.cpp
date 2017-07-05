#include "service_rest.h"

RestService::RestService(const char *user, const char *pass,
                         const uint16_t port) {
    if (user != NULL && pass != NULL && strlen(user) > 0 && strlen(pass) > 0) {
        acc = (char *) malloc(sizeof(char) * (1 + strlen(user)));
        strcpy(acc, user);
        passwd = (char *) malloc(sizeof(char) * (1 + strlen(pass)));
        strcpy(passwd, pass);
        login_id = generate_login_id();
    }
    web_server = new ESP8266WebServer(port);
    const char *headerkeys[] = {"Cookie"};
    web_server->collectHeaders(headerkeys, 1);
    web_server->onNotFound([this]() { on_not_found(); });
    web_server->on("/login", [=]() {
        if (!valid_credentials()) {
            on_invalid_credentials();
        } else if (HTTP_GET == web_server->method()) {
            String header("HTTP/1.1 301 OK\r\nSet-Cookie: LOGINID=");
            header += login_id;
            header += "\r\nLocation: /\r\nCache-Control: no-cache\r\n\r\n";
            web_server->sendContent(header);
        } else
            on_not_found();
    });
    web_server->on("/logout", [=]() {
        if (!valid_credentials())
            return on_invalid_credentials();
        if (HTTP_GET == web_server->method()) {
            web_server->sendContent("HTTP/1.1 301 OK\r\nSet-Cookie: "
                                            "LOGINID=0\r\nLocation: /\r\nCache-Control: "
                                            "no-cache\r\n\r\n");
        } else
            on_not_found();
    });
    web_server->begin();
}

void RestService::on_not_found() {
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

uint32_t RestService::generate_login_id() {
    uint32_t id = ESP.getChipId() + ESP.getVcc();
    for (int i = strlen(acc); i >= 0; i--) {
        id += acc[i];
    }
    for (int i = strlen(passwd); i >= 0; i--) {
        id += passwd[i];
    }
    return id;
}

void RestService::on_invalid_credentials() {
    File
    file = SPIFFS.open(HTML_LOGIN
    ".gz", "r");
    if (file) {
        web_server->streamFile(file, RESP_HTML);
        file.close();
    } else {
        web_server->requestAuthentication();
    }
}

bool RestService::valid_credentials() {
    if (acc == NULL || passwd == NULL) {
        return true;
    }
    if (web_server->hasHeader("Cookie")) {
        String valid_cookie("LOGINID=");
        valid_cookie += login_id;
        if (web_server->header("Cookie").indexOf(valid_cookie) != -1) {
            return true;
        }
    }
    return web_server->authenticate(acc, passwd);
}

void RestService::add_handler(const char *uri, HTTPMethod method,
                              const char *resp_type,
                              RestServiceFunction handler,
                              const bool authentication) {
    web_server->on(uri, [=]() {
        Log::println("Recieved on %s %s", authentication ? "Auth" : "NAuth", uri);
        if (authentication && !valid_credentials())
            return on_invalid_credentials();
        if (method == HTTP_ANY || method == web_server->method()) {
            web_server->send(200, resp_type, handler(web_server->arg("plain")));
        } else
            on_not_found();
    });
}

void RestService::add_handler_file(const char *uri, HTTPMethod method,
                                   const char *resp_type, const char *file_name,
                                   const bool authentication) {
    web_server->on(uri, [=]() {
        Log::println("Recieved on %s %s", authentication ? "Auth" : "NAuth", uri);
        if (authentication && !valid_credentials())
            return on_invalid_credentials();
        if (method == HTTP_ANY || method == web_server->method()) {
            File file = SPIFFS.open(file_name, "r");
            if (file) {
                web_server->streamFile(file, resp_type);
                file.close();
            } else {
                Log::println("File %s cannot be opened.", file_name);
                on_not_found();
            }
        } else
            on_not_found();
    });
}

void RestService::cycle_routine() {
    web_server->handleClient();
    MDNS.update();
}
