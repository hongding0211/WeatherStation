#pragma once

#include "Querier.h"

/*
 *
 * Acquire location based on IP address.
 *
 */

class LocationClient : public Querier {
private:
    void updateURL();

    const String apiKey = "";
    String ipAddr;
    String region = "N/A";
    String regionCode = "N/A";
public:
    String getRegion();

    LocationClient();

    void setIPAddr(String _ipAddr);

    unsigned char inquiryLocation();

    String getRegionCode();
};
