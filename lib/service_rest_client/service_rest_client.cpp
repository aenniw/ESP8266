#include "service_rest_client.h"

AsyncRestClient::AsyncRestClient() {
    requests = new std::queue<REST_REQUEST *>();
}

void AsyncRestClient::_send(REST_REQUEST *request) {
    HTTPClient client;
    Log::println(request->host);
    client.begin(request->host);
    int code = 0;
    switch (request->type) {
        case GET:
            code = client.GET();
            break;
        case POST:
            code = client.POST(request->payload);
            break;
    }
    if (code == HTTP_CODE_OK) {
        if (request->callback) {
            (*request->callback)(client.getString());
        }
    } else {
        Log::println("Error http get: %d", code);
        Log::println("Error: %s", client.errorToString(code).c_str());
    }
    client.end();
}

REST_REQUEST *AsyncRestClient::delete_request(REST_REQUEST *request) {
    delete request->host;
    delete request->payload;
    delete request->callback;
    return request;
}

AsyncRestClient *AsyncRestClient::getInstance() {
    static auto *client = new AsyncRestClient();
    return client;
}

void AsyncRestClient::get(const char *host, const RequestCallback &callback = nullptr) {
    if (!host) return;

    auto *request = new REST_REQUEST;
    request->host = new char[strlen(host) + 1];
    request->payload = nullptr;
    request->callback = nullptr;
    strcpy(request->host, host);
    if (callback) {
        request->callback = new RequestCallback(callback);
    }
    request->type = GET;

    requests->push(request);
}

void AsyncRestClient::post(const char *host, const char *payload, const RequestCallback &callback = nullptr) {
    if (!host) return;

    auto *request = new REST_REQUEST;
    request->host = new char[strlen(host) + 1];
    strcpy(request->host, host);
    request->payload = nullptr;
    request->callback = nullptr;
    if (payload && strlen(payload)) {
        request->payload = new char[strlen(payload) + 1];
        strcpy(request->payload, payload);
    }
    if (callback) {
        request->callback = new RequestCallback(callback);
    }
    request->type = GET;

    requests->push(request);
}

void AsyncRestClient::cycle_routine() {
    if (!requests->empty()) {
        REST_REQUEST *request = requests->front();
        _send(request);
        requests->pop();
        delete delete_request(request);
    }
}

AsyncRestClient::~AsyncRestClient() {
    while (!requests->empty()) {
        REST_REQUEST *request = requests->front();
        requests->pop();
        delete delete_request(request);
    }
}