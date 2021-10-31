#pragma once

#include "Querier.h"

/*
 *
 * Acquire public IPv4 address.
 *
 */

class IPInquiry : public Querier {
private:
    String ipAddr;
public:
    IPInquiry();

    String getIPAddr();

    unsigned char inquiryIP();
};
