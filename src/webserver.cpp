#include "webserver.h"

// ----------------------------------------------------------------------------
// Web server
// ----------------------------------------------------------------------------


// Locals
String errorReplacementPage = "";
// WebServer
AsyncWebServer server(my_http_server_port);
DNSServer dnsServer;


void onRootRequest(AsyncWebServerRequest *request) {
    const char* errorMsg = "";

    if (!FSmounted) {
        errorMsg = "Error: filesystem not mounted correctly";
    } else if (!config.wifiMode) {
        errorMsg = "Error: cannot parse config file";
    } else {
        errorMsg = "how did you get here?";
    }

    errorReplacementPage = String(index_html);
    errorReplacementPage.replace(errorPlaceHolder, errorMsg);
    errorReplacementPage.replace(errorCodePlaceHolder, "500");
    request->send(500, "text/html", errorReplacementPage);
}

void notFound(AsyncWebServerRequest *request) {
    errorReplacementPage = String(index_html);
    errorReplacementPage.replace(errorPlaceHolder, "Error: Page Not Found");
    errorReplacementPage.replace(errorCodePlaceHolder, "404");
    request->send(404, "text/html", errorReplacementPage);
}

void initWebServer() {
    if (FSmounted) {
        server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    } else {
        server.on("/", onRootRequest);
    }

    server.onNotFound(notFound);
    server.begin();
    
    if (!MDNS.begin(device)) {
        Serial.println("Error setting up MDNS responder!");
    }
    MDNS.addService("http", "tcp", 80);

    // ArduinoOTA.begin();
}

void handlehostname() {
    if (config.wifiMode == WIFI_AP) {
        dnsServer.processNextRequest();
        MDNS.update();
    } else {
        MDNS.update();
    }
}
