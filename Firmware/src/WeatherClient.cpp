#include "WeatherClient.h"
#include <Arduino_JSON.h>


WeatherClient::WeatherClient() {
    // icon map
    // Group 2xx: Thunderstorm
    iconMap[200] = "0";
    iconMap[201] = "0";
    iconMap[202] = "0";
    iconMap[210] = "0";
    iconMap[211] = "0";
    iconMap[212] = "0";
    iconMap[221] = "0";
    iconMap[220] = "0";
    iconMap[230] = "0";
    iconMap[231] = "0";
    iconMap[232] = "0";
    // Group 3xx: Drizzle
    iconMap[300] = "Q";
    iconMap[301] = "Q";
    iconMap[302] = "Q";
    iconMap[310] = "Q";
    iconMap[311] = "Q";
    iconMap[312] = "Q";
    iconMap[313] = "Q";
    iconMap[314] = "Q";
    iconMap[300] = "Q";
    // Group 5xx: Rain
    iconMap[500] = "Q";
    iconMap[501] = "R";
    iconMap[502] = "R";
    iconMap[503] = "R";
    iconMap[504] = "R";
    iconMap[511] = "R";
    iconMap[520] = "R";
    iconMap[521] = "R";
    iconMap[522] = "R";
    iconMap[531] = "R";
    // Group 6xx: Snow
    iconMap[600] = "W";
    iconMap[601] = "W";
    iconMap[602] = "W";
    iconMap[611] = "W";
    iconMap[612] = "W";
    iconMap[613] = "W";
    iconMap[615] = "W";
    iconMap[616] = "W";
    iconMap[620] = "W";
    iconMap[621] = "W";
    iconMap[622] = "W";
    // Group 7xx: Atmosphere
    iconMap[701] = "M";
    iconMap[711] = "M";
    iconMap[721] = "M";
    iconMap[731] = "M";
    iconMap[741] = "M";
    iconMap[751] = "M";
    iconMap[761] = "M";
    iconMap[762] = "M";
    iconMap[771] = "M";
    iconMap[781] = "M";
    // Group 800: Clear
    iconMap[800] = "B";
    // Group 80x: Cloud
    iconMap[801] = "H";
    iconMap[802] = "N";
    iconMap[803] = "Y";
    iconMap[804] = "Y";


    // WeatherCondition map
    // Group 2xx: Thunderstorm
    weatherCondtionMap[200] = "Thunderstorm";
    weatherCondtionMap[201] = "Thunderstorm";
    weatherCondtionMap[202] = "Thunderstorm";
    weatherCondtionMap[210] = "Thunderstorm";
    weatherCondtionMap[211] = "Thunderstorm";
    weatherCondtionMap[212] = "Thunderstorm";
    weatherCondtionMap[221] = "Thunderstorm";
    weatherCondtionMap[220] = "Thunderstorm";
    weatherCondtionMap[230] = "Thunderstorm";
    weatherCondtionMap[231] = "Thunderstorm";
    weatherCondtionMap[232] = "Thunderstorm";
    // Group 3xx: Drizzle
    weatherCondtionMap[300] = "Light Drizzle";
    weatherCondtionMap[301] = "Drizzle";
    weatherCondtionMap[302] = "Heavy Drizzle";
    weatherCondtionMap[310] = "Drizzle Rain";
    weatherCondtionMap[311] = "Drizzle Rain";
    weatherCondtionMap[312] = "Drizzle Rain";
    weatherCondtionMap[313] = "Drizzle Rain";
    weatherCondtionMap[314] = "Drizzle Rain";
    weatherCondtionMap[321] = "Shower Drizzle";
    // Group 5xx: Rain
    weatherCondtionMap[500] = "Light Rain";
    weatherCondtionMap[501] = "Rain";
    weatherCondtionMap[502] = "Heavy Rain";
    weatherCondtionMap[503] = "Heavy Rain";
    weatherCondtionMap[504] = "Extreme Rain";
    weatherCondtionMap[511] = "Freezing Rain";
    weatherCondtionMap[520] = "Shower Rain";
    weatherCondtionMap[521] = "Shower Rain";
    weatherCondtionMap[522] = "Shower Rain";
    weatherCondtionMap[531] = "Shower Rain";
    // Group 6xx: Snow
    weatherCondtionMap[600] = "Light Snow";
    weatherCondtionMap[601] = "Snow";
    weatherCondtionMap[602] = "Heavy Snow";
    weatherCondtionMap[611] = "Sleet";
    weatherCondtionMap[612] = "Shower Sleet";
    weatherCondtionMap[613] = "Shower Sleet";
    weatherCondtionMap[615] = "Rain Snow";
    weatherCondtionMap[616] = "Rain Snow";
    weatherCondtionMap[620] = "Shower Snow";
    weatherCondtionMap[621] = "Shower Snow";
    weatherCondtionMap[622] = "Shower Snow";
    // Group 7xx: Atmosphere
    weatherCondtionMap[701] = "Mist";
    weatherCondtionMap[711] = "Smoke";
    weatherCondtionMap[721] = "Haze";
    weatherCondtionMap[731] = "Sand";
    weatherCondtionMap[741] = "Fog";
    weatherCondtionMap[751] = "Sand";
    weatherCondtionMap[761] = "Dust";
    weatherCondtionMap[762] = "Volcanic Ash";
    weatherCondtionMap[771] = "Squalls";
    weatherCondtionMap[781] = "Tornado";
    // Group 800: Clear
    weatherCondtionMap[800] = "Clear";
    // Group 80x: Cloud
    weatherCondtionMap[801] = "Few Clouds";
    weatherCondtionMap[802] = "Clouds";
    weatherCondtionMap[803] = "Clouds";
    weatherCondtionMap[804] = "Overcast";


    paramsLive["appid"] = appID;
    paramsLive["units"] = "metric";

    paramsOneCall["appid"] = appID;
    paramsOneCall["units"] = "metric";
    paramsOneCall["exclude"] = "current,minutely,hourly,alerts";

    host = "api.openweathermap.org";
    childName = "WeatherClient";
}

const WeatherCondition &WeatherClient::getLiveWeather() const {
    return liveWeather;
}

const std::vector<WeatherCondition> &WeatherClient::getHourlyWeather() const {
    return hourlyWeather;
}

const std::vector<WeatherCondition> &WeatherClient::getDailyWeather() const {
    return dailyWeather;
}

void WeatherClient::syncWeatherLive() {
    syncLiveSucess = false;

    if (getResponse2(urlLive, paramsLive) != 1)
        return;
    JSONVar obj = JSON.parse(response);

    (int)obj["cod"] == 200 ? syncLiveSucess = true : syncLiveSucess = false;
    liveWeather.time = (long) obj["dt"];
    liveWeather.sunrise = (long) obj["sys"]["sunrise"];
    liveWeather.sunset = (long) obj["sys"]["sunset"];
    liveWeather.temp = (double) obj["main"]["temp"];
    liveWeather.tempL = (double) obj["main"]["temp_min"];
    liveWeather.tempH = (double) obj["main"]["temp_max"];
    liveWeather.weatherID = (int) obj["weather"][0]["id"];
//    liveWeather.weatherCondition = JSON.stringify(obj["weather"][0]["description"]);
    liveWeather.weatherCondition = (const char*)obj["weather"][0]["description"];
    if (obj.hasOwnProperty("rain")) {
        liveWeather.willRain = true;
        liveWeather.rainVolumes = (double) obj["rain"]["1h"];
    } else {
        liveWeather.willRain = false;
    }
}

void WeatherClient::syncWeatherAll() {
    if (getResponse2(urlOneCall, paramsOneCall) != 1)
        return;
    auto obj = JSON.parse(response);
    auto daily = obj["daily"];

//    dailyWeather.empty();
    dailyWeather.clear();

    for(int i=0;i<daily.length();i++){
        WeatherCondition w;
        w.time = (long)daily[i]["dt"];
        w.tempL = (double)daily[i]["temp"]["min"];
        w.tempH = (double)daily[i]["temp"]["max"];
        w.sunrise = (long)daily[i]["sunrise"];
        w.sunset = (long)daily[i]["sunset"];
        w.weatherID = (int)daily[i]["weather"][0]["id"];
//        w.weatherCondition = JSON.stringify(daily[i]["description"]);
//        w.weatherCondition = (const char*)daily[i]["description"];
        if(daily[i].hasOwnProperty("rain")){
            w.willRain = true;
            w.rainVolumes = (double) obj["rain"]["1h"];
        } else {
            w.willRain = false;
        }

        dailyWeather.push_back(w);
    }
}

void WeatherClient::setCurrentCity(const String &currentCity) {
    WeatherClient::currentCity = currentCity;

    paramsLive["q"] = currentCity;

    // 调用一次 API 来获取经纬度
    if (getResponse2(urlLive, paramsLive) == 1) {
        JSONVar obj = JSON.parse(response);
        latitude = JSON.stringify(obj["coord"]["lat"]);
        longitude = JSON.stringify(obj["coord"]["lon"]);

        paramsOneCall["lon"] = longitude;
        paramsOneCall["lat"] = latitude;

        locationName = obj["name"];
    }
}

const String &WeatherClient::getLocationName() const {
    return locationName;
}


