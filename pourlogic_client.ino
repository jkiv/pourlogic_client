// See LICENSE.txt for license details.

// General
#include <Arduino.h>
#include <SPI.h>

// Ethernet
//#include <Client.h>
#include <Ethernet.h>
//#include <Server.h>
//#include <Udp.h>
#include <EEPROM.h>
#include <String.h>
#include <sha256.h> // SHA Hashing (https://github.com/jkiv/Cryptosuite/)

// PourLogic
#include "config.h"
#include "pin_config.h"

#include "StreamUtil.h"
#include "HexString.h"
#include "ParallaxRFID.h"
#include "FlowMeter.h"
#include "Valve.h"
#include "PourLogicClient.h"

static FlowMeter flowMeter;
static ParallaxRFID rfidReader;
static Valve valve;
static PourLogicClient client(SETTINGS_CLIENT_KEY);

static byte mac[6] = SETTINGS_ETHERNET_MAC;

void fail(char code) {
  Serial.print(code);
}

//!<Setup the PourLogic controller environment and settings
void setup() {   
    // Start serial communications
    Serial.begin(ParallaxRFID::RFID_BAUD);
    
    // Set up RFID reader
    rfidReader.begin(RFID_ENABLE_PIN);
    
    // Set up flow meter
    flowMeter.begin(FLOW1_PIN, FLOW1_INTERRUPT);
    
    // Set up valve
    valve.begin(VALVE1_PIN);
    
#ifdef SETTINGS_ETHERNET_USE_DHCP
    // Set up ethernet
    while (Ethernet.begin(mac) == 0) {
      delay(5000);  
    }
#else
    // TODO static IP
#endif

}

/*! \brief Handle PourLogic day to day operations.
 */
void loop()
{
  int maxVolume_mL;
  int volume_mL;
  String tagData;
  
  // Wait a second before serving next person
  delay(1000);
  
  // Wait for RFID read
  if (!rfidReader.readRFID(tagData)) {
    return; // Read failed
  }
  
  // Get the max volume the patron can pour
  if (!client.requestMaxVolume(tagData, maxVolume_mL)) {
    return; // Request failed
  }
  
  // Can the patron pour at all?
  if (maxVolume_mL == 0) {
    return; // The user cannot pour
  }
  
  // Open valve
  valve.open();
  
  // Start reading flow meter
  volume_mL = flowMeter.readVolume_mL(maxVolume_mL);
  
  // Close valve
  valve.close();

  // Report pour volume
  if (volume_mL > 0) {
    client.reportPouredVolume(tagData, volume_mL);
  }
}
