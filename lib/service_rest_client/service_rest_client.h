#ifndef ESP8266_PROJECTS_ROOT_SERVICE_REST_CLIENT_H
#define ESP8266_PROJECTS_ROOT_SERVICE_REST_CLIENT_H

#include <queue>
#include <commons.h>
#include <service_log.h>
#include <ESP8266HTTPClient.h>

typedef void (*RequestCallback)(const String &);

typedef enum {
    GET, POST
} REST_METHOD;

typedef struct {
    REST_METHOD type;
    char *host;
    char *payload;
    RequestCallback *callback;
} REST_REQUEST;

class AsyncRestClient : public Service {
private:
    std::queue<REST_REQUEST *> *requests = nullptr;
protected:
    AsyncRestClient();

    void _send(REST_REQUEST *request);

    REST_REQUEST *delete_request(REST_REQUEST *request);

public:
    static AsyncRestClient *getInstance();

    void get(const char *host, const RequestCallback &callback = nullptr);

    void post(const char *host, const char *payload = nullptr, const RequestCallback &callback = nullptr);

    void cycle_routine() override;

    ~AsyncRestClient();

};

#endif //ESP8266_PROJECTS_ROOT_SERVICE_REST_CLIENT_H
