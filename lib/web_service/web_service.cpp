#include <web_service.h>

ESP8266WebServer server(80);

void setup_web_server(const char *host_name) {
  server.onNotFound([]() { server.send(404, "text/plain", "Page not found"); });
  server.on("/", []() {
    server.send(200, "text/html", get_file_content("/index.html"));
  });
  // Start TCP (HTTP) server
  server.begin();

  // Add service to MDNS-SD
  if (host_name != NULL)
    if (MDNS.begin(host_name))
      Serial.println("Error setting up MDNS responder!");
    else
      MDNS.addService("http", "tcp", 80);
}

void handle_web_server_client() { server.handleClient(); }
