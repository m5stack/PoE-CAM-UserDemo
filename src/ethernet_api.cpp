#include "ethernet_api.h"


bool eth_init() {
    SPI.begin(ETH_PIN_CLK, ETH_PIN_MISO, ETH_PIN_MOSI, -1);
    SPI.setFrequency(ETH_SPI_SPEED);
    Ethernet.init(ETH_PIN_CS);

    uint8_t base_mac_addr[6];
    uint8_t w5500_mac_addr[6];
    esp_efuse_mac_get_default(base_mac_addr);
    esp_derive_local_mac(w5500_mac_addr, base_mac_addr);
    Serial.printf("PoE use MAC: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                  w5500_mac_addr[0], w5500_mac_addr[1], w5500_mac_addr[2],
                  w5500_mac_addr[3], w5500_mac_addr[4], w5500_mac_addr[5]);

    if (Ethernet.begin(w5500_mac_addr) != 1) {
        Serial.println("Error getting IP address via DHCP, trying again...");
        return false;
    }
    return true;
}

String eth_get_ip() {
    return Ethernet.localIP().toString();
}
