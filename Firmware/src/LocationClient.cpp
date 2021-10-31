#pragma once

#include "LocationClient.h"
#include <Arduino_JSON.h>


LocationClient::LocationClient() {
    childName = "LocaitionClient";
    host = "restapi.amap.com";
}

void LocationClient::setIPAddr(String _ipAddr) {
    ipAddr = _ipAddr;
}

void LocationClient::updateURL() {
    url = "/v3/ip?ip=" + ipAddr + "&output=json&key=" + apiKey;
}

unsigned char LocationClient::inquiryLocation() {
    if (ipAddr == "") {
        region = "N/A";
        return 0;
    } else {
        updateURL();
        getResponse();
        response.trim();
        JSONVar obj = JSON.parse(response);
        String s = ""; // buffer
        s = obj["status"];
        if (s.indexOf("1") != -1) {
            s = obj["city"];
            s.replace("市", "");
            region = s;
            regionCode = obj["adcode"];
            debuggerLog(ipAddr + " -> " + region);
            return 1;
        } else {
            region = "N/A";
            return 0;
        }
    }
}

String LocationClient::getRegion() {
    return region;
}

String LocationClient::getRegionCode() {
    return regionCode;
}