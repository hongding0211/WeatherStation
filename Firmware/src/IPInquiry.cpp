#pragma once

#include "IPInquiry.h"
#include <Arduino_JSON.h>

IPInquiry::IPInquiry() {
    childName = "IPInquiry";
    host = "api.ipify.org";
    url = "/?format=jsonp&callback=ok";
}

String IPInquiry::getIPAddr() {
    return ipAddr;
}

unsigned char IPInquiry::inquiryIP() {
    getResponse();
    response.trim();
    if (response.indexOf("ok") != -1) {
        response.replace("ok", "");
        response.replace("(", "");
        response.replace(")", "");
        response.replace(";", "");

        JSONVar obj = JSON.parse(response);
        String s = ""; // buffer
        s = obj["ip"];
        ipAddr = s;
        return 1;
    } else {
        ipAddr = "127.0.0.1";
        return 0;
    }
}