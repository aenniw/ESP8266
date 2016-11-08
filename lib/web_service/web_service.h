#ifndef WEBSERVICE_H_
#define WEBSERVICE_H_

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <file_system.h>

void setup_web_server(const char *host_name);

void handle_web_server_client();

#endif /* WEBSERVICE_H_ */
