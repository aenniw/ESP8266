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


void RestService::set_auth(const char *user, const char *pass) {
    if (user != NULL && strlen(user) > 0) {
        if (acc) free(acc);
        acc = (char *) malloc(sizeof(char) * (1 + strlen(user)));
        strcpy(acc, user);
    }
    if (pass != NULL && strlen(pass) > 0) {
        if (passwd) free(passwd);
        passwd = (char *) malloc(sizeof(char) * (1 + strlen(pass)));
        strcpy(passwd, pass);
    }
}

RestService::RestService(const char *user, const char *pass, const uint16_t port) {
    set_auth(user, pass);
    web_server = new ESP8266WebServer(port);
    web_server->onNotFound([this]() { on_not_found(); });
    web_server->begin();
}

void RestService::on_not_found() {
#ifdef __CORS__
    if (web_server->method() == HTTP_OPTIONS) {
        web_server->sendHeader("Access-Control-Allow-Origin", "*");
        web_server->sendHeader("Access-Control-Max-Age", "10000");
        web_server->sendHeader("Access-Control-Allow-Methods", "PUT,POST,GET,OPTIONS");
        web_server->sendHeader("Access-Control-Allow-Headers", "*");
        web_server->send(204);
        return;
    }
#endif
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

void RestService::on_invalid_credentials() {
#ifdef __CORS__
    if (web_server->method() == HTTP_OPTIONS) {
        web_server->sendHeader("Access-Control-Allow-Origin", "*");
        web_server->sendHeader("Access-Control-Max-Age", "10000");
        web_server->sendHeader("Access-Control-Allow-Methods", "PUT,POST,GET,OPTIONS");
        web_server->sendHeader("Access-Control-Allow-Headers", "*");
        web_server->send(204);
        return;
    }
#endif
    web_server->requestAuthentication();
}

bool RestService::valid_credentials() {
    if (acc == NULL || passwd == NULL) {
        return true;
    }
    return web_server->authenticate(acc, passwd);
}

void RestService::log_update_error() {
    StreamString str;
    Update.printError(str);
    Log::println(str);
}

void RestService::add_handler_update(const char *uri, const bool authentication) {
    web_server->on(uri, HTTP_POST, [&]() {
        if (authentication && !valid_credentials())
            return on_invalid_credentials();
        if (Update.hasError()) {
            web_server->send(200, RESP_JSON, JSON_RESP_NOK);
        } else {
            web_server->client().setNoDelay(true);
            web_server->send_P(200, RESP_JSON, JSON_RESP_OK);
            delay(100);
            web_server->client().stop();
            ESP.restart();
        }
    }, [&]() {
        if (authentication && !valid_credentials())
            return on_invalid_credentials();

        int uploadDestination = U_FLASH;
        if (web_server->hasArg("mode")) {
            uploadDestination = web_server->arg("mode").toInt();
        }

        HTTPUpload &upload = web_server->upload();
        if (upload.status == UPLOAD_FILE_START) {
            WiFiUDP::stopAll();
            Log::println("Update: %s mode: %d", upload.filename.c_str(), uploadDestination);
            uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
            if (!Update.begin(maxSketchSpace, uploadDestination)) {
                log_update_error();
            }
        } else if (upload.status == UPLOAD_FILE_WRITE && !Update.hasError()) {
            Log::print(".");
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                log_update_error();
            }
        } else if (upload.status == UPLOAD_FILE_END && !Update.hasError()) {
            if (Update.end(true)) {
                Log::println("Update Success: %u\nRebooting...", upload.totalSize);
            } else {
                log_update_error();
            }
        } else if (upload.status == UPLOAD_FILE_ABORTED) {
            Update.end();
            Log::println("Update was aborted");
        }
        delay(0);
    });
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
#ifdef __CORS__
            web_server->sendHeader("Access-Control-Allow-Origin", "*");
#endif
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
                web_server->sendHeader("Cache-Control", "max-age=" CACHE_TTL);
#ifdef __CORS__
            web_server->sendHeader("Access-Control-Allow-Origin", "*");
#endif
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
#ifdef __CORS__
            web_server->sendHeader("Access-Control-Allow-Origin", "*");
#endif
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
                web_server->sendHeader("Cache-Control", "max-age=" CACHE_TTL);
#ifdef __CORS__
            web_server->sendHeader("Access-Control-Allow-Origin", "*");
#endif
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
                    web_server->sendHeader("Cache-Control", "max-age=" CACHE_TTL);
#ifdef __CORS__
                web_server->sendHeader("Access-Control-Allow-Origin", "*");
#endif
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
                    web_server->sendHeader("Cache-Control", "max-age=" CACHE_TTL);
#ifdef __CORS__
                web_server->sendHeader("Access-Control-Allow-Origin", "*");
#endif
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
                    web_server->sendHeader("Cache-Control", "max-age=" CACHE_TTL);
#ifdef __CORS__
                web_server->sendHeader("Access-Control-Allow-Origin", "*");
#endif
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
