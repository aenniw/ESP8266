#include "service_rest.h"

static bool strmatch(const char *s, const char *p, const int n, const int m) {
    if (m - (p[m - 1] == '?' ? 1 : 0) > n) return false;
    if (m <= 0) return n <= 0;
    bool wc = false;
    int j = 0;
    for (int i = 0; i < n; i++) {
        if (i > 0) {
            if (p[j] == '/') return strmatch(s + i, p + j, n - i, m - j);
            if (s[i] == '/') {
                return strmatch(s + i, p + j + (p[j] == '?' ? 0 : 1),
                                n - i, m - j - (p[j] == '?' ? 0 : 1));
            }
        }
        if (p[i] == '+') wc = true;
        if (wc) continue;
        if (s[i] != p[j] && p[j] != '?') return false;
        if (s[i] != '/' && p[j] == '?') return false;
        if (j < m) j++;
    }
    return true;
}


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
            String resp = handler(web_server->arg("plain"));
            web_server->send(200, resp_type, resp);
        } else
            on_not_found();
    });
}

void RestService::add_handler_stream(const char *uri, HTTPMethod method,
                                     const char *resp_type, FStream *st,
                                     const bool authentication, const bool caching) {
    web_server->on(uri, [=]() {
        Log::println("Recieved on %s %s", authentication ? "Auth" : "NAuth", uri);
        if (authentication && !valid_credentials())
            return on_invalid_credentials();
        if (method == HTTP_ANY || method == web_server->method()) {
            if (caching)
                web_server->sendHeader("Cache-Control", "max-age="CACHE_TTL);
            web_server->streamFile(*st, resp_type);
            st->rewind();
        } else
            on_not_found();
    });
}

void
RestService::add_handler_wc(const char *uri, HTTPMethod method, const char *resp_type, WcRestServiceFunction handler,
                            const bool authentication) {
    web_server->addHandler(new WcRequestHandler([=]() {
        Log::println("Recieved on %s %s", authentication ? "Auth" : "NAuth", uri);
        if (authentication && !valid_credentials())
            return on_invalid_credentials();
        if (method == HTTP_ANY || method == web_server->method()) {
            String args = web_server->arg("plain");
            String resp = handler(args, web_server->uri());
            web_server->send(200, resp_type, resp);
        } else
            on_not_found();
    }, uri, method));
}

void RestService::add_handler_wc_stream(const char *uri, HTTPMethod method, const char *resp_type, FStream *fs,
                                        const bool authentication, const bool caching) {
    web_server->addHandler(new WcRequestHandler([=]() {
        Log::println("Recieved on %s %s", authentication ? "Auth" : "NAuth", uri);
        if (authentication && !valid_credentials())
            return on_invalid_credentials();
        if (method == HTTP_ANY || method == web_server->method()) {
            if (caching)
                web_server->sendHeader("Cache-Control", "max-age="CACHE_TTL);
            web_server->streamFile(*fs, resp_type);
            fs->rewind();
        } else
            on_not_found();
    }, uri, method));
}

void RestService::add_handler_file(const char *uri, HTTPMethod method,
                                   const char *resp_type, const char *file_name,
                                   const bool authentication, const bool caching) {
    web_server->on(uri, [=]() {
        Log::println("Recieved on %s %s", authentication ? "Auth" : "NAuth", uri);
        if (authentication && !valid_credentials())
            return on_invalid_credentials();
        if (method == HTTP_ANY || method == web_server->method()) {
            File file = SPIFFS.open(file_name, "r");
            if (file) {
                if (caching)
                    web_server->sendHeader("Cache-Control", "max-age="CACHE_TTL);
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

void RestService::add_handler_wc_file(const char *uri, HTTPMethod method,
                                      const char *resp_type, const char *file_name,
                                      const bool authentication, const bool caching) {
    web_server->addHandler(new WcRequestHandler([=]() {
        Log::println("Recieved on %s %s", authentication ? "Auth" : "NAuth", uri);
        if (authentication && !valid_credentials())
            return on_invalid_credentials();
        if (method == HTTP_ANY || method == web_server->method()) {
            File file = SPIFFS.open(file_name, "r");
            if (file) {
                if (caching)
                    web_server->sendHeader("Cache-Control", "max-age="CACHE_TTL);
                web_server->streamFile(file, resp_type);
                file.close();
            } else {
                Log::println("File %s cannot be opened.", file_name);
                on_not_found();
            }
        } else
            on_not_found();
    }, uri, method));
}

void RestService::add_handler_wc_file(const char *uri, HTTPMethod method,
                                      const char *resp_type, WcUriTranslator t,
                                      const bool authentication, const bool caching) {
    web_server->addHandler(new WcRequestHandler([=]() {
        Log::println("Recieved on %s %s", authentication ? "Auth" : "NAuth", uri);
        if (authentication && !valid_credentials())
            return on_invalid_credentials();
        if (method == HTTP_ANY || method == web_server->method()) {
            String file_name = t(web_server->uri());
            File file = SPIFFS.open(file_name, "r");
            if (file) {
                if (caching)
                    web_server->sendHeader("Cache-Control", "max-age="CACHE_TTL);
                web_server->streamFile(file, resp_type);
                file.close();
            } else {
                Log::println("File %s cannot be opened.", file_name.c_str());
                on_not_found();
            }
        } else
            on_not_found();
    }, uri, method));
}

void RestService::cycle_routine() {
    web_server->handleClient();
    MDNS.update();
}

WcRequestHandler::WcRequestHandler(const ESP8266WebServer::THandlerFunction fn, const String &u, const HTTPMethod m) {
    handler = fn;
    method = m;
    uri = new char[(uri_l = u.length()) + 1];
    strcpy(uri, u.c_str());
}

bool WcRequestHandler::canHandle(HTTPMethod m, String uri_) {
    if (method != HTTP_ANY && method != m) {
        return false;
    }
    return strmatch(uri_.c_str(), uri, uri_.length(), uri_l);
}

bool WcRequestHandler::handle(ESP8266WebServer &server, HTTPMethod requestMethod, String requestUri) {
    if (!canHandle(requestMethod, requestUri))
        return false;
    handler();
    return true;
}
