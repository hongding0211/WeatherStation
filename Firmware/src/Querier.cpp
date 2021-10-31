#include "Querier.h"

unsigned char Querier::getResponse() {
    int count = retryTimes;
    while (true) {
        delay(0);
        httpClient.begin(host, port, url);
        httpCode = httpClient.GET();
        count--;
        if (httpCode == 200) {
            response = httpClient.getString();
            httpClient.end();
            return 1;
        } else {
            response = "N/A";
            if (count) {
                delay(3000);
            } else {
                httpClient.end();
                return 0;
            }
        }
    }
}


unsigned char Querier::getResponse2(const String &url2, std::map<String, String> &params) {
    String requestURL = url2 + "?";
    for (auto &p : params)
        requestURL += p.first + "=" + p.second + "&";

    int count = retryTimes;
    while (true) {
        delay(0);
        debuggerLog("Try to connect server -> " + host + requestURL);
        httpClient.begin(host, port, requestURL);
        httpCode = httpClient.GET();
        count--;
        debuggerLog("HTTP_CODE = " + String(httpCode) + " Size: " + String(httpClient.getSize()));
        if (httpCode == 200) {
            response = httpClient.getString();
            httpClient.end();
            return 1;
        } else {
            response = "N/A";
            if (count) {
                delay(3000);
            } else {
                httpClient.end();
                return 0;
            }
        }
    }
}
