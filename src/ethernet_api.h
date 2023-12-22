
#ifndef _ETH_API_H_
#define _ETH_API_H_

#include <SPI.h>
#include <M5_Ethernet.h>
#include "ethernet_pins.h"

bool eth_init();
String eth_get_ip();
#endif