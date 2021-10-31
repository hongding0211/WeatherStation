#pragma once

#include "Querier.h"
#include "vector"
#include "map"

struct WeatherCondition {
    uint32 time = 0;
    uint32 sunrise = 0;
    uint32 sunset = 0;
    float temp = 0;
    float tempL = 0;
    float tempH = 0;
    int weatherID = 800;
    String weatherCondition = "Clear";
    bool willRain = false;
    float rainVolumes = 0.0f;
};

/*
 *
 * Fetching weather info using OpenWeather API.
 *
 * https://openweathermap.org
 *
 */

class WeatherClient : public Querier {
private:
    const String appID = "";
    std::map<String, String> paramsLive;
    std::map<String, String> paramsOneCall;
    const String urlLive = "/data/2.5/weather";
    const String urlOneCall = "/data/2.5/onecall";

    String currentCity = "Shanghai";
    String latitude = "31.231";
    String longitude = "121.472";
    String locationName = "Shanghai";
    WeatherCondition liveWeather;
    std::vector<WeatherCondition> hourlyWeather;
    std::vector<WeatherCondition> dailyWeather;

public:

    std::map<int, String> iconMap;
    std::map<int, String> weatherCondtionMap;

    bool syncLiveSucess = false;

    WeatherClient();

    const WeatherCondition &getLiveWeather() const;

    const std::vector<WeatherCondition> &getHourlyWeather() const;

    const std::vector<WeatherCondition> &getDailyWeather() const;

    void syncWeatherLive();

    void syncWeatherAll();

    void setCurrentCity(const String &currentCity);

    const String &getLocationName() const;
};
