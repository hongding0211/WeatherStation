#pragma once

#include "IPInquiry.h"
#include "LocationClient.h"
#include "QRCode.h"
#include "SH1106.h"
#include "WeatherStationFonts.h"
#include "WeatherStationImages.h"
#include "Wire.h"
#include "WeatherClient.h"
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <Updater.h>
#include <NTPClient.h>
#include <Ticker.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ctime>
#include <cmath>
#include <vector>

struct config_type {
    char saved = 1;
    char savedRegion[100];
};

/******** AP Info ********/
const String apPwd = "";
const IPAddress apIP(192, 168, 4, 1);
const IPAddress apGateway(192, 168, 4, 1);
const IPAddress apSubMask(255, 255, 255, 0);
String apSSID = "WeatherStation";

/******** STA Info ********/
String staSSID = "";
String staPwd = "";

/******** Flags ********/
bool apConnected = false;
bool wifiConnected = false;
bool readyForWeatherUpdateLive = true;
bool readyFroWeatherUpdateFull = true;
bool frameDrawn = false;
bool fullUpdateTag = false;
bool showQRCode = true;
bool doneConfig;

/******** Callbacks ********/
WiFiEventHandler apConnectedHandler;
WiFiEventHandler staConnectLostHandler;

/******** Params ********/
const unsigned int OLED_W = 128;
const unsigned int OLED_H = 64;
const unsigned short utcOffset = 8;
const unsigned long ntpUpdateInterval = 1000000 * 30;   // 30s
const int UPDATE_INTERVAL_LIVE = 60;                    // Update every 1 min
const int UPDATE_INTERVAL_ALL = 60 * 60 * 6;            // Update every 6 hours
const int I2C_DISPLAY_ADDRESS = 0x3c;
const int SDA_PIN = 12;
const int SDC_PIN = 13;
const int framesCnt = 3;
const char *host = "esp8266-webupdate";
config_type config;
time_t dt;
int currentFrame = 0;
String locatedRegion = "";
String customRegion = "";
String currentRegion = "N/A";

/******** Clients ********/
WiFiUDP ntpUDP;
WeatherClient weatherClient;
LocationClient locationClient;
IPInquiry ipInquiry;
NTPClient ntpClient(ntpUDP, "ntp1.aliyun.com", 0);
SH1106 display(I2C_DISPLAY_ADDRESS, SDA_PIN, SDC_PIN);
Ticker liveweatherUpdateTicker, frameTicker, apTicker, fullUpdateTagTicker;
ESP8266WebServer webServer(80);
const char indexHTML[] = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1.0, minimum-scale=1.0, maximum-scale=1.0, user-scalable=no\" /><title>Weather Station</title><style>body{ font-family: sans-serif; display: flex; justify-content: center;} #main{ margin-left: 10px; margin-right: 10px; width: 500px; color: #262626;} #title-box{ display: flex; padding: 20px 0 10px 0; align-items: center;} .title-content{ margin-left: 15px;} #title{ font-size: x-large; font-weight: bold;} #box{ display: flex; align-items: center; justify-content: space-between; padding: 20px 20px 20px 20px; border-radius: 3px; margin-bottom: 25px; box-shadow: 0px 1.6px 3.6px rgb(0 0 0 / 13%), 0px 0px 2.9px rgb(0 0 0 / 11%);} #wifi-icon{ margin-right: 25px;} #box-left{ display: flex; align-items: center;} #net-info{ display: flex; flex-direction: column; justify-content: center;} #net-info-status{ display: flex; align-items: center; color: #107c10; font-size: small; font-weight: 600;} #net-info-status-notok{ display: flex; align-items: center; color: #b62c2c; font-size: small; font-weight: 600;} .status-icon{ width: 12px; margin-right: 4px;} .item{ display: flex; align-items: center; justify-content: space-between; font-weight: 600; margin-bottom: 15px; cursor: pointer;} .item-icon{ margin-right: 20px; width: 14px;} .item-arrow-btn{ display: inline-flex; align-items: center; justify-content: center; width: 25px; height: 25px; cursor: pointer; background-color: transparent; border: transparent; transition: all 0.2s;} .item-arrow-btn:hover{ border-radius: 2.5px; background-color: #f2f2f2;} .item-arrow{ width: 8px;} #footer{ display: block; text-align: center; margin-top: 40px; text-decoration: none; font-size: small; color: gray;} #params-box{ padding: 20px; border-radius: 4px; margin-bottom: 25px; box-shadow: 0px 1.6px 3.6px rgb(0 0 0 / 13%), 0px 0px 2.9px rgb(0 0 0 / 11%);} .para{ display: flex; justify-content: space-between; align-items: center; margin-bottom: 30px;} .para-name{ font-weight: 600; font-size: 0.9em; margin-left: 5px; min-width: 40%; max-width: 40%;} .para-value{ border: 1px solid #b6b6b6; padding: 0 11px; border-radius: 2px; height: 32px; width: 100%; transition: all 0.2s;} .para-value:hover{ border-color: #909090;} #save-btn-container{ display: flex; justify-content: flex-end; margin-top: 30px;} #save-btn{ cursor: pointer; height: 32px; padding: 0 20px; transition: all 0.1s; border: 1px solid transparent; border-radius: 3px; color: #fff; background-color: #0078d4;} @media (prefers-color-scheme: dark){ body{ background: #222;} #main{ color: #eee;} #box{ background-color: #202020;} .fa-chevron-right{ color: #fff;} .item-arrow-btn{ color:#eee} .item-arrow-btn:hover{ background-color: #5e5e5e;} #net-info-status{ color: #139113;} #net-info-status-notok{ color: #bb3030;} .para-value{ background-color: #333333; border-color: #737373; color: #fff;}} </style></head><body><div id=\"main\"><div id=\"title-box\"><span class=\"title-content\" id=\"title\">Weather Station Neo</span></div><hr style=\"margin-bottom: 25px;\"><div id=\"box\"><div id=\"box-left\"><svg id=\"wifi-icon\" height=\"32px\" aria-hidden=\"true\" focusable=\"false\" data-prefix=\"fas\" data-icon=\"wifi\" class=\"svg-inline--fa fa-wifi fa-w-20\" role=\"img\" xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 640 512\"><path fill=\"currentColor\" d=\"M634.91 154.88C457.74-8.99 182.19-8.93 5.09 154.88c-6.66 6.16-6.79 16.59-.35 22.98l34.24 33.97c6.14 6.1 16.02 6.23 22.4.38 145.92-133.68 371.3-133.71 517.25 0 6.38 5.85 16.26 5.71 22.4-.38l34.24-33.97c6.43-6.39 6.3-16.82-.36-22.98zM320 352c-35.35 0-64 28.65-64 64s28.65 64 64 64 64-28.65 64-64-28.65-64-64-64zm202.67-83.59c-115.26-101.93-290.21-101.82-405.34 0-6.9 6.1-7.12 16.69-.57 23.15l34.44 33.99c6 5.92 15.66 6.32 22.05.8 83.95-72.57 209.74-72.41 293.49 0 6.39 5.52 16.05 5.13 22.05-.8l34.44-33.99c6.56-6.46 6.33-17.06-.56-23.15z\"></path></svg><div id=\"net-info\"><span style=\"font-weight: 600;\"></span><span style=\"font-size: small; margin-top:5px;\"></span><span style=\"font-size: x-small; color: gray; margin-left:1px;\"></span></div></div><div><div id=\"net-info-status\"><span>Connected</span></div></div></div><div id='params-box'><div id=\"para1\" class=\"para\"><span class=\"para-name\">可用网络</span><select id=\"wifi-list\" class=\"para-value\"><option>获取中...</option></select></div><div id=\"para2\" class=\"para\"><span class=\"para-name\">密码</span><input id=\"wifi-password\" class=\"para-value\"></div><div id=\"para3\" class=\"para\"><span class=\"para-name\">自定义地点</span><input id='custom-region' class=\"para-value\"></div><div id=\"save-btn-container\"><button id=\"save-btn\" type=\"button\">保存</button></div></div><div style=\"margin-left: 15px;\"><div id=\"update-firmware\" class=\"item\"><div class=\"item-title\">版本更新 </div><button class=\"item-arrow-btn\">></button></div><footer><a href=\"https://github.com/HongDing97/WeatherStation\" id=\"footer\" target=\"_blank\">© Hong</a></footer></div><script>info={},ssids=[];var request=new XMLHttpRequest,request2=new XMLHttpRequest;function writeInfo(){var e=document.getElementById(\"net-info\");e.children[0].textContent=info.ssid,e.children[1].textContent=info.ipaddr,e.children[2].textContent=\"当前天气定位：\"+info.useRegion;var t=document.getElementById(\"net-info-status\");info.isConnect?(t.innerHTML=\"<span>已连接</span>\",t.id=\"net-info-status\"):(t.innerHTML=\"<span>未连接</span>\",t.id=\"net-info-status-notok\")}function prepareLinks(){document.getElementById(\"update-firmware\").onclick=()=>{window.open(\"/update\",\"_self\")},document.getElementById(\"save-btn\").onclick=()=>{let e=document.getElementById(\"wifi-list\"),t=e[e.selectedIndex].text,n=document.getElementById(\"wifi-password\").value,s=document.getElementById(\"custom-region\").value,o=new XMLHttpRequest;setTimeout((()=>{alert(\"保存成功！\")}),200),o.open(\"POST\",\"/commit\",!0),o.setRequestHeader(\"Content-type\",\"application/x-www-form-urlencoded\"),o.send(`ssid=${t}&pwd=${n}&region=${s}`)}}function updateNetInfo(){request.open(\"GET\",\"/getnetinfo\",!0),request.onreadystatechange=()=>{4==request.readyState&&(info=JSON.parse(request.responseText),writeInfo())},request.send(null)}function updateSSIDs(){request2.open(\"GET\",\"/getssids\",!0),request2.onreadystatechange=()=>{if(4==request2.readyState){ssids=JSON.parse(request2.responseText);var e=document.getElementById(\"wifi-list\");for(s of(e.innerHTML=\"\",ssids))e.innerHTML+=`<option>${s}</option>`}},request2.send(null)}window.onload=()=>{updateNetInfo(),updateSSIDs(),prepareLinks()}; </script></body></html>";
const char updateHtml[] = "<!DOCTYPE html><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1.0, minimum-scale=1.0, maximum-scale=1.0, user-scalable=no\" /><title>Firmware Update</title><style>body{ font-family: sans-serif; display: flex; justify-content: center;} #main{ margin-left: 10px; margin-right: 10px; width: 500px; color: #262626;} #title-box{ display: flex; padding: 20px 0 10px 0; align-items: center;} #title{ font-size: x-large; font-weight: bold;} #back-btn{ width: 30px; height: 30px; border: transparent; background-color: transparent; cursor: pointer;} #box{ padding: 20px; border-radius: 4px; box-shadow: 0px 1.6px 3.6px rgb(0 0 0 / 13%), 0px 0px 2.9px rgb(0 0 0 / 11%);} #box-top{ display: flex; align-items: center; width: 100%; margin-bottom: 15px;} #fileopen-icon{ width: 18px; margin-right: 8px;} #file{ display: inline-flex; padding: 0px 10px; align-items: center; justify-content: space-between; cursor: pointer; width: 100%; height: 35px; transition: all 0.1s; border: 1px solid #b6b6b6; border-radius: 2px; color: #2a2a2a; background-color: #fff;} #file:hover{ border-color: #909090;} #box-btm{ display: flex; align-items: center; padding-left: 25px; justify-content: space-between;} #upload{ cursor: pointer; height: 35px; padding: 0 20px; transition: all 0.1s; border: 1px solid #b6b6b6; border-radius: 2px; color: #2a2a2a; background-color: #fff;} #upload:hover{ border-color: #909090;} #upload:active:enabled{ border-color: #cecece;} #file-name{ font-size: small;} @media (prefers-color-scheme: dark){ body{ background: #222;} #main{ color: #eee;} #box{ background-color: #202020;} #back-btn{ color: #fff;} #box-btm{ display: flex; flex-direction: row-reverse;} #file{ background-color: #333333; border-color: #737373; color: #fff;} #upload{ background-color: #333; color: #fff; border-color: #737373;} #upload:active:enabled{ border-color: #5c5c5c;} #opt-icon{ color: #fff;} #progress{ background-color: #0875c9;} #progress-index{ color: #777;}} </style></head><body><div id=\"main\"><div id=\"title-box\"><button id=\"back-btn\" onclick=\"window.open('/','_self')\" style=\"font-size: 1.5em;\"><</button><span id=\"title\">Firmware Update</span></div><hr style=\"margin-bottom: 25px;\"><div id=\"box\"><div id=\"box-top\"><form id=\"form\" method='POST' action='/upload' enctype='multipart/form-data' style=\"display: none;\"><input id=\"file-input\" type='file' name='update' onchange=\"onFileInputClick(this.files)\"><input type='submit' value='Update'></form><div id=\"file\"><span id=\"file-name\">Upload File Here</span>···</div></form></div><div id=\"box-btm\"><button id=\"upload\" type=\"button\">Upload</button></div></div></div><script>var uploadBtn, selectFile, form, file, fileName, fileChoosed=!1; function linkElem(){ uploadBtn=document.getElementById(\"upload\"), selectFile=document.getElementById(\"file\"), form=document.getElementById(\"form\"), file=document.getElementById(\"file-input\"), fileName=document.getElementById(\"file-name\")} function setHandler(){ uploadBtn.onclick=onBtnClick, selectFile.onclick=onSelectFile} function onBtnClick(){ return fileChoosed && (setTimeout((()=>{ alert(\"Uploading...Please wait until the device is fully rebooted.\")}), 100), form.submit()), !1} function onFileInputClick(e){ return fileName.innerText=e[0].name, fileName.style.fontWeight=600, fileChoosed=!0, !1} function onSelectFile(){ return file.click(), !1} window.onload=()=>{ linkElem(), setHandler()};</script></body>";

/******** Functions ********/
void drawFrame1();

void drawFrame2();

void drawFrame3();

bool initWiFi();

void wifiConfig();

void startWebServer();

void onAPConnected(const WiFiEventSoftAPModeStationConnected &evt);

void drawQRCODE(int x, int y);

void drawProgress(int percentage, String label);

void updateWeatherLive();

void updateWeatherFull();

void setReadyForWeatherUpdate();

void closeAP();

String getDay(int n);

void drawForecast(int x, int y, int dayIndex);

int getHourFromTimestamp(long t);

int getMinFromTimestamp(long t);

String getFormattedStrOfTimestamp(long t);

int getWeekDayFromTimestamp(long t);

int roundUp(float f);
