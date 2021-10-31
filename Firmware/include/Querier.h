#pragma once

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "Debugger.h"
#include <vector>
#include <map>

/*
 *
 * Send http requests.
 *
 */

class Querier : public Debugger {
protected:
    int retryTimes = 3;
    String host;
    int port = 80;
    String url;
    String response;
    HTTPClient httpClient;
    int httpCode;

    unsigned char getResponse();

    // New Version
    unsigned char getResponse2(const String &url2, std::map<String, String> &params);
};
