// See LICENSE.txt for license details.

// General
#include <Arduino.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <String.h>

// Ethernet
#include <Ethernet.h>
#include <sha1.h> // SHA Hashing (https://github.com/jkiv/Cryptosuite/)

// PourLogic
#include "config.h"
#include "pin_config.h"

#include "StreamUtil.h"
#include "HTTPUtil.h"
#include "HexString.h"

#include "RFID.h"
#include "FlowMeter.h"
#include "Valve.h"
#include "PourLogicClient.h"


#ifdef RFID_USE_SOFTWARE_SERIAL
static SoftwareSerial RFIDSerial(RFID_RX_PIN, RFID_TX_PIN);
static RFID_EM41000 rfidReader(RFIDSerial, RFID_ENABLE_PIN);
#else
static RFID_EM41000 rfidReader(Serial, RFID_ENABLE_PIN);
#endif
static FlowMeter flowMeter(FLOW1_PIN, FLOW1_INTERRUPT);
static Valve valve(VALVE1_PIN);
static PourLogicClient client(SETTINGS_CLIENT_ID, SETTINGS_CLIENT_KEY);
static byte mac[6] = SETTINGS_ETHERNET_MAC; // MAC address of ethernet shield

//!<Setup the PourLogic controller environment and settings
void setup() {

#ifdef RFID_USE_SOFTWARE_SERIAL
  RFIDSerial.begin(RFID_BAUD_RATE);
  Serial.begin(9600);
#else
  Serial.begin(RFID_BAUD_RATE);
#endif
    
#ifdef SETTINGS_ETHERNET_USE_DHCP
    while (Ethernet.begin(mac) == 0) {
      delay(1000);
    }
#else
    Ethernet.begin(mac, SETTINGS_ETHERNET_IP);
#endif

  // TODO Grab any settings from the server that might be of interest (e.g. flow conversion?)
}

/*! \brief Handle PourLogic day to day operations.
 */
void loop()
{
  int max_volume_in_mL = 0;
  float poured_volume_in_mL = 0.f;
  String tag_data;

  // Restrict loop timing
  delay(1000);
  
  // Wait for RFID
  if (!rfidReader.readRFID(tag_data)) {
    return; // RFID read timed-out or failed.
  }
  
  // Get the max volume the patron can pour
  if (!client.requestMaxVolume(tag_data, max_volume_in_mL)) {
    return; // Request failed
  }
  
  // Can the patron pour?
  if (max_volume_in_mL == 0) {
    return; // The patron cannot pour
  }
  
  // Open valve
  valve.open();
  
  // Read flow meter
  poured_volume_in_mL = flowMeter.readVolume_mL(max_volume_in_mL);

  // Close valve
  valve.close();
  
  // Send pour data to be logged on the server
  if (poured_volume_in_mL > 0) {
    client.reportPouredVolume(tag_data, poured_volume_in_mL);
  }
}
