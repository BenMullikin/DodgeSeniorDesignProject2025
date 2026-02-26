#include <Arduino.h>
#include "lwIOLinkMaster.hpp"
#include "driver/phy/tiol112/tiol112.hpp"
#include "port.hpp"

using namespace lwIOLink;
using namespace lwIOLink::driver::phy;

// ============================================
// Configuration
// ============================================

TIOL112::tiol_cfg phyConfig;

TIOL112* phy;
Port* port1;
Port* portList[1];
Master* master;

// ============================================
// Setup Function
// ============================================

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(10); }
    
    Serial.println("=================================");
    Serial.println("TIOL1115EVM IO-Link Master Demo");
    Serial.println("=================================");

    // Initialize config
    phyConfig.tx_en = 4;
    phyConfig.fault_pin = 5;
    phyConfig.serial = &Serial2;

    // Initialize Serial2 for IO-Link
    Serial2.begin(230400, SERIAL_8E1, 16, 17);
    
    // Create instances
    phy = new TIOL112(&phyConfig);
    port1 = new Port(phy, 1);
    portList[0] = port1;
    master = new Master(portList, 1);

    // Initialize the master
    if (master->begin()) {
        Serial.println("IO-Link Master initialized successfully");
    } else {
        Serial.println("Failed to initialize IO-Link Master");
        while (1) { delay(1000); }
    }

    Serial.println("Ready. Waiting for IO-Link devices...");
}

// ============================================
// Main Loop
// ============================================

void loop() {
    phy->RecieveTask();
    delay(10);
}