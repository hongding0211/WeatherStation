#include "main.h"

void setup() {
// put your setup code here, to run once:
    Serial.begin(115200);

    /******** OLED Settings ********/
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setContrast(255);
    display.clear();
    display.display();

    /******** AP Settings ********/
    String macAddr = WiFi.macAddress();
    macAddr.replace(":", "");
    apSSID += "_" + macAddr.substring(0, 4);

    /******** Start WiFi ********/
    bool wifiConnectResult = initWiFi();
    startWebServer();
    if (!wifiConnectResult)
        wifiConfig();
    WiFi.mode(WIFI_AP_STA);

    /******** NTP Client ********/
    display.clear();
    drawProgress(10, "Init NTP...");
    display.display();
    ntpClient.begin();

    /******** IP Locating ********/
    display.clear();
    drawProgress(15, "Locating");
    display.display();
    ipInquiry.inquiryIP();
    locationClient.setIPAddr(ipInquiry.getIPAddr());

    display.clear();
    drawProgress(25, "Locating");
    display.display();
    locationClient.inquiryLocation();
    locatedRegion = locationClient.getRegion();

    /******** Read saved region ********/
    EEPROM.begin(1024);
    char *p = (char *) (&config);
    for (int i = 0; i < sizeof(config); i++) {
        *(p + i) = EEPROM.read(i);
    }
    EEPROM.commit();
    if (config.saved == 1) {
        Serial.println("[Main Sys] Record exisits");
        customRegion = String(config.savedRegion);
        currentRegion = customRegion;
    } else {
        currentRegion = locatedRegion;
    }
    Serial.println("[Main Sys] Using region -> " + currentRegion);

    /******** Tickers ********/
    liveweatherUpdateTicker.attach(UPDATE_INTERVAL_LIVE, []() {
        setReadyForWeatherUpdate();
    });
    liveweatherUpdateTicker.active();

    frameTicker.attach(5, []() {
        currentFrame = (++currentFrame) % framesCnt;
        frameDrawn = false;
    });
    frameTicker.active();

    apTicker.once(60 * 5, []() { closeAP(); });
    apTicker.active();

    /******** Ready to Go ********/
    display.clear();
    drawProgress(60, "Fetching data");
    display.display();

    weatherClient.setCurrentCity(currentRegion);
    weatherClient.syncWeatherLive();
    if(!weatherClient.syncLiveSucess) {          // Fail to use custom region
        weatherClient.setCurrentCity(locatedRegion);
        currentRegion = locatedRegion;
        weatherClient.syncWeatherLive();
    }

    display.clear();
    drawProgress(80, "Fetching data");
    display.display();

    display.clear();
    drawProgress(100, "Done");
    display.display();
    delay(500);
}

void loop() {
// put your main code here, to run repeatedly:

    webServer.handleClient();

    ntpClient.update();
    dt = ntpClient.getEpochTime();

    if (getHourFromTimestamp(dt) % 6 == 0 && getMinFromTimestamp(dt) == 0 && !fullUpdateTag) {
        readyForWeatherUpdateLive = true;
        readyFroWeatherUpdateFull = true;
        fullUpdateTag = true;
        fullUpdateTagTicker.once(70, []() {
            fullUpdateTag = false;
        });
        fullUpdateTagTicker.active();
    }

    if (readyForWeatherUpdateLive && WiFi.status() == WL_CONNECTED) {
        updateWeatherLive();
    }
    if (readyFroWeatherUpdateFull && WiFi.status() == WL_CONNECTED) {
        updateWeatherFull();
    }

    if (!frameDrawn) {
        if (currentFrame == 0)
            drawFrame1();
        else if (currentFrame == 1)
            drawFrame2();
        else if (currentFrame == 2)
            drawFrame3();
        else;
        frameDrawn = true;
    }
}

bool initWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    unsigned short count = 0;
    int percentage = 0;
    WiFi.begin();
    int dot = 0;
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        percentage = (int) (((float) count / 30.0) * 100.0);
        display.clear();
        drawProgress(percentage, "Loading...");
        display.display();
        count++;
        delay(1000);
        if (count > 30) {
            Serial.println("");
            Serial.println("WIFI RCONNECTING TIMEOUT");
            display.drawString(OLED_W / 2, 45, "[X] Unable to connect");
            display.display();
            wdt_reset();
            delay(2000);
            return false;
        }
    }
    Serial.println("");
    Serial.println("RECONNECTED TO -> " + WiFi.SSID() + " IP -> " +
                   WiFi.localIP().toString());
    display.clear();
    drawProgress(100, "Connect to WiFi...");
    display.drawString(OLED_W / 2, 45, "[OK] " + WiFi.SSID());
    display.display();
    return true;
}

void wifiConfig() {
    WiFi.softAPConfig(apIP, apGateway, apSubMask);
    WiFi.softAP(apSSID);
    apConnectedHandler = WiFi.onSoftAPModeStationConnected(
            [&](const WiFiEventSoftAPModeStationConnected &e) { onAPConnected(e); });

    display.clear();
    display.drawRect(3, 3, OLED_W - 6 - 1, 20 - 2);
    display.drawString(OLED_W / 2, 6, "Step 1");
    display.drawString(OLED_W / 2, OLED_H / 2 - 10, "Connect to");
    display.drawString(OLED_W / 2, OLED_H / 2, "'WeatherStation'");
    display.drawString(OLED_W / 2, OLED_H / 2 + 10, "via your device");
    display.display();

    while (!apConnected) {
        delay(1000);
        wdt_reset();
    }

    showQRCode = true;

    doneConfig = false;
    while (!doneConfig || !wifiConnected) {
        wdt_reset();
        if (showQRCode) {
            display.clear();
            display.drawRect(5, OLED_H / 2 - 2 - 3 - 10, 55, 20 - 2);
            display.drawString(OLED_W / 4, OLED_H / 2 - 2 - 10, "Step 2");
            drawQRCODE(75, 11);
            display.display();
            delay(1000);
        }
        webServer.handleClient();
    }
    delay(1500);
}

void startWebServer() {
    /******** Home ********/
    webServer.on("/", []() {
        webServer.send(200, "text/html", indexHTML);
    });

    /******** Submit Changes ********/
    webServer.on("/commit", []() {
        if (webServer.hasArg("ssid") && webServer.hasArg("pwd") &&
            webServer.hasArg("region")) {
            staSSID = webServer.arg("ssid");
            staPwd = webServer.arg("pwd");
            customRegion = webServer.arg("region");
            doneConfig = true;
            showQRCode = false;
        }
        String donestr;
        webServer.send(200, "text/plain", "DONE");
        Serial.println("[Web Server] SSID -> " + staSSID + " PWD -> " + staPwd);

        unsigned short count = 0;
        int percentage = 0;
        WiFi.begin(staSSID, staPwd);
        while (WiFi.status() != WL_CONNECTED) {
            Serial.print(".");
            display.clear();
            percentage = (int) (((float) count / 30.0) * 100.0);
            drawProgress(percentage, "Connect to WiFi...");
            display.display();
            delay(1000);
            count++;
            if (count > 30) {
                Serial.println("");
                display.drawString(OLED_W / 2, 45, "[X] Wrong SSID or password");
                display.display();

                webServer.stop();
                webServer.begin();
                delay(3000);
                showQRCode = true;
                return;
            }
        }
        Serial.println("");

        currentRegion = customRegion;
        weatherClient.setCurrentCity(currentRegion);
        weatherClient.syncWeatherLive();
        if(!weatherClient.syncLiveSucess) {          // Fail to use custom region
            weatherClient.setCurrentCity(locatedRegion);
            currentRegion = locatedRegion;
            weatherClient.syncWeatherLive();
        }

        /******** write config ********/
        config.saved = 1;
        char buff[100];
        currentRegion.toCharArray(buff, sizeof(buff));
        strcpy(config.savedRegion, buff);
        EEPROM.begin(1024);
        char *p = (char *) (&config);
        for (int i = 0; i < sizeof(config); i++) {
            EEPROM.write(i, *(p + i));
        }
        EEPROM.commit();
        Serial.println("[Main Sys] Config written");
        wifiConnected = true;
        Serial.println("");
        Serial.println("[MainSys] CONNECTED TO -> " + WiFi.SSID() + " IP -> " +
                       WiFi.localIP().toString());

        display.clear();
        drawProgress(100, "Connecting to WiFi");
        display.drawString(OLED_W / 2, 45, "[OK] " + WiFi.SSID());
        display.display();
        Serial.println("");
    });

    /******** Update Firmware ********/
    webServer.on("/update", []() {
        webServer.send(200, "text/html", updateHtml);
    });

    /******** Upload ********/
    webServer.on("/upload", HTTP_POST, []() {
        webServer.sendHeader("Connection", "close");
        webServer.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        ESP.restart();
    }, []() {
        HTTPUpload& upload = webServer.upload();
        if (upload.status == UPLOAD_FILE_START) {
            Serial.setDebugOutput(true);
            WiFiUDP::stopAll();
            Serial.printf("Update: %s\n", upload.filename.c_str());
            uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
            if (!Update.begin(maxSketchSpace)) { //start with max available size
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_END) {
            if (Update.end(true)) { //true to set the size to the current progress
                Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
            } else {
                Update.printError(Serial);
            }
            Serial.setDebugOutput(false);
        }
        yield();
    });

    /******** Get SSIDs ********/
    webServer.on("/getssids", [&] {
        int cnt = WiFi.scanNetworks();
        std::vector<String> networks;
        for(int i=0;i<cnt;i++){
            if(WiFi.SSID(i)=="")
                continue;
            networks.push_back(WiFi.SSID(i));
        }
        String res = "[";
        for (auto &network:networks) {
            res += "\"" + network + "\",";
        }
        res[res.length() - 1] = ']';
        webServer.send(200, "text/plain", res);
    });

    /******** Get Net Info ********/
    webServer.on("/getnetinfo", [&] {
        String isCon, ssid, ipaddr;
        isCon = WiFi.isConnected() ? "true" : "false";
        ssid = WiFi.isConnected() ? WiFi.SSID() : "N/A";
        ipaddr = WiFi.isConnected() ? WiFi.localIP().toString() : "192.168.4.1";
        String res = "{\"ssid\": \"" + ssid
                     + "\",\"ipaddr\": \"" + ipaddr
                     + "\",\"macaddr\": \"" + WiFi.macAddress()
                     + "\",\"isConnect\": " + isCon
                     + ",\"useRegion\": \"" + currentRegion
                     + "\"}";
        webServer.send(200, "text/plain", res);
    });

    /******** Redirect ********/
    webServer.on("/done", []() {
        webServer.send(200, "text/html", "DONE");
    });

    webServer.begin();
    Serial.println("HTTP server started");
}

void onAPConnected(const WiFiEventSoftAPModeStationConnected &evt) {
    apConnected = true;
}

void drawQRCODE(int x, int y) {
    int px = x;
    int py = y;
    for (int i = 0; i < QRCodeSize; i++) {
        for (int j = 0; j < QRCodeSize; j++) {
            if (QRCode[i][j]) {
                display.setPixel(px, py);
                display.setPixel(px + 1, py);
                display.setPixel(px, py + 1);
                display.setPixel(px + 1, py + 1);
            }
            px += 2;
        }
        px = x;
        py += 2;
    }
}

void drawProgress(int percentage, String label) {
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_10);
    display.drawString(OLED_W / 2, 10, label);
    display.drawRect(10, 28, 108, 12);
    display.fillRect(12, 30, 104 * percentage / 100, 9);
}

void drawFrame1() {

    display.clear();

    /******** Icon ********/
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(Meteocons_0_42);
    String weatherIcon = weatherClient.iconMap[weatherClient.getLiveWeather().weatherID];
    int weatherIconWidth = display.getStringWidth(weatherIcon);
    display.drawString(30 - weatherIconWidth / 2, 10, weatherIcon);

    /******** Weather Condition ********/
    display.setFont(ArialMT_Plain_10);
    display.drawString(60, 10, weatherClient.weatherCondtionMap[weatherClient.getLiveWeather().weatherID]);

    /******** Temp ********/
    display.setFont(ArialMT_Plain_24);
    char buff[50];
    sprintf(buff, "%d°C", roundUp(weatherClient.getLiveWeather().temp));
    int tempWid = display.getStringWidth(buff);
    display.drawString(60, 20, buff);

    display.display();

}

void drawFrame2() {

    display.clear();

    bool isSunrise;
    bool day;
    int riseH[2], riseM[2];
    int setH[2], setM[2];
    auto dailyWeathers = weatherClient.getDailyWeather();
    riseH[0] = getHourFromTimestamp(dailyWeathers[0].sunrise);
    riseH[1] = getHourFromTimestamp(dailyWeathers[1].sunrise);
    riseM[0] = getMinFromTimestamp(dailyWeathers[0].sunrise);
    riseM[1] = getMinFromTimestamp(dailyWeathers[1].sunrise);

    setH[0] = getHourFromTimestamp(dailyWeathers[0].sunset);
    setH[1] = getHourFromTimestamp(dailyWeathers[1].sunset);
    setM[0] = getMinFromTimestamp(dailyWeathers[0].sunset);
    setM[1] = getMinFromTimestamp(dailyWeathers[1].sunset);

    int cH = getHourFromTimestamp(dt);
    int cM = getMinFromTimestamp(dt);
    int cTotalM = cH * 60 + cM;

    // 0 ~ sunrise
    if (cH >= 0 && cTotalM <= (riseH[0] * 60 + riseM[0])) {
        isSunrise = true;
        day = 0;
    }
        // sunrise ~ sunset
    else if (cTotalM > (riseH[0] * 60 + riseM[0]) && cTotalM <= (setH[0] * 60 + setM[0])) {
        isSunrise = false;
        day = 0;
    }
        // sunset ~ 24
    else {
        isSunrise = true;
        day = 1;
    }

    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_10);
    display.drawString(32, 0, "Temp L");
    display.drawString(32, 30, "Temp H");
    display.drawString(96, 0, "Rain");
    if (isSunrise) {
        display.drawString(96, 30, "Sunrise");
    } else {
        display.drawString(96, 30, "Sunset");
    }

    display.setFont(ArialMT_Plain_16);
    display.drawString(32, 10, String(roundUp(weatherClient.getLiveWeather().tempL)) + "°");
    display.drawString(32, 40, String(roundUp(weatherClient.getLiveWeather().tempH)) + "°");
    if (weatherClient.getLiveWeather().willRain) {
        char buff[20];
        sprintf(buff, "%.1fmm", weatherClient.getLiveWeather().rainVolumes);
        display.drawString(96, 10, buff);
    } else {
        display.drawString(96, 10, "None");
    }
    if (isSunrise) {
        display.drawString(96, 40, getFormattedStrOfTimestamp(dailyWeathers[day].sunrise));
    } else {
        display.drawString(96, 40, getFormattedStrOfTimestamp(dailyWeathers[day].sunset));
    }

    display.display();

}

void drawFrame3() {
    display.clear();
    if (weatherClient.getDailyWeather().size() > 3) {
        for (int i = 0; i < 3; i++)
            drawForecast(44 * i, 0, 1 + i);
    }
    display.display();
}

void drawForecast(int x, int y, int dayIndex) {
    auto w = weatherClient.getDailyWeather()[dayIndex];

    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_10);
    int weekday = getWeekDayFromTimestamp(w.time);
    String day = getDay(weekday);
    day = day.substring(0, 3);
    day.toUpperCase();
    display.drawString(x + 20, y, day);

    display.setFont(Meteocons_0_21);
    display.drawString(x + 20, y + 15, weatherClient.iconMap[w.weatherID]);

    display.setFont(ArialMT_Plain_16);
    display.drawString(x + 20, y + 37, String(roundUp(w.tempL)) + "/" + String(roundUp(w.tempH)));

    display.setTextAlignment(TEXT_ALIGN_LEFT);
}

void updateWeatherLive() {
    weatherClient.syncWeatherLive();
    readyForWeatherUpdateLive = false;
    delay(1000);
}

void updateWeatherFull() {
    display.clear();
    drawProgress(50, "Fetching from servers");
    display.display();

    weatherClient.syncWeatherAll();

    display.clear();
    drawProgress(100, "Sync done");
    display.display();

    readyFroWeatherUpdateFull = false;
    delay(500);
}

void setReadyForWeatherUpdate() {
    readyForWeatherUpdateLive = true;
}

void closeAP() {
    WiFi.mode(WIFI_STA);
}

String getDay(int n) {
    switch (n) {
        case 0:
            return "Sunday";
            break;
        case 1:
            return "Monday";
            break;
        case 2:
            return "Tuesday";
            break;
        case 3:
            return "Wednesday";
            break;
        case 4:
            return "Thursday";
            break;
        case 5:
            return "Friday";
            break;
        case 6:
            return "Saturday";
            break;
        default:
            break;
    }
}

int getHourFromTimestamp(long t) {
    t += 60 * 60 * utcOffset;
    auto tm = gmtime(&t);
    return (tm->tm_hour);
}

int getMinFromTimestamp(long t) {
    t += 60 * 60 * utcOffset;
    auto tm = gmtime(&t);
    return tm->tm_min;
}

int getWeekDayFromTimestamp(long t) {
    t += 60 * 60 * utcOffset;
    auto tm = gmtime(&t);
    return tm->tm_wday;
}

String getFormattedStrOfTimestamp(long t) {
    t += 60 * 60 * utcOffset;
    auto tm = gmtime(&t);
    char buf[10];
    sprintf(buf,"%02d:%02d",tm->tm_hour,tm->tm_min);
    return String(buf);
}

int roundUp(float f) {
    return (int) round(f);
}
