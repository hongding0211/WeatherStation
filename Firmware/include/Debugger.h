#pragma once

#include "ESP8266WiFi.h"

/*
 *
 * Debugger Log
 *
 */

class Debugger {
protected:
    String childName;

    void debuggerLog(String str) {
        Serial.println("[" + childName + "] " + str);
    }
};