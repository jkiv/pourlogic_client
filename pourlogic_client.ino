// See LICENSE.txt for license details.

#include "config.h"

// General
#include <Arduino.h>
#include <SPI.h>
//#include <WProgram.h>
//#include <avr/interrupt.h>
//#include <avr/pgmspace.h>

// Ethernet
#include <Client.h>
#include <Ethernet.h>
#include <Server.h>
#include <Udp.h>

// EEPROM
#include <EEPROM.h>

// SD
//#include <SD.h>

// Strings
#include <String.h>

// SHA Hashing (https://github.com/jkiv/Cryptosuite/)
#include <sha256.h>

// PourLogic
//#include "BasicOO.h"
#include "StringConversion.h"
#include "IPUtil.h"
#include "StreamUtil.h"
#include "HexString.h"
#include "ParallaxRFID.h"
#include "FlowMeter.h"
#include "Valve.h"
#include "PourLogicClient.h"
//#include "Settings.h" // TODO

static FlowMeter flowMeter;
static ParallaxRFID rfidReader;
static Valve valve;

static byte mac[6] = SETTINGS_ETHERNET_MAC;

//!< Called when failure occurs (i.e. often) 
void fail(const char* msg = 0) {
  // Print the fail message
  if (msg) {
    Serial.println(msg);
  }
}

//!< Called when success occurs
void success() {
}

//!<Setup the PourLogic controller environment and settings
void setup() {   
    // Start serial communications
    Serial.begin(ParallaxRFID::RFID_BAUD);

    // Initialize SD card
    //pinMode(SD_REQUIRED_PIN, OUTPUT);
    //SD.begin(SD_CS_PIN);
    
    // Load pourlogic settings
    //settings.fromFile("pourlogic.ini");
    
    // Set up RFID reader
    rfidReader.begin(RFID_ENABLE_PIN);
    
    // Set up flow meter
    flowMeter.begin(FLOW1_PIN, FLOW1_INTERRUPT);
    
    // Set up valve
    valve.begin(VALVE1_PIN);
    
    // Set up ethernet
    while (Ethernet.begin(mac) == 0) {
      fail("Failed to configure Ethernet using DHCP");
      delay(5000);  
    }
    
    // Set up piezo
    //piezo.begin(PIEZO_PIN);
}

/*! \brief Handle pourlogic day to day operations.
 */
void loop()
{
  PourLogicClient client(SETTINGS_CLIENT_KEY);
  
  uint32_t maxVolume_mL;
  uint32_t volume_mL;
  String tagData;
  
  // Wait a second before serving next person
  delay(1000);

  Serial.println("Waiting for RFID...");
  
  // Wait for RFID read
  tagData = rfidReader.readRFID();
  
  Serial.println(tagData);
  
  if (tagData.length() == 0)
  {
    fail("Bad RFID.");
    return;
  }
  
  // Request pour
  if (!client.connect(SETTINGS_SERVER_IP, SETTINGS_SERVER_PORT))
  {
    fail("Could not connect to server...");
    return;
  }
  
  if (!client.sendPourRequest(tagData))
  {
    fail("Pour request failed (send).");
    return;
  }
  
  if (!client.getPourRequestResponse(maxVolume_mL))
  {
    fail("Pour request failed (recv).");
    return;
  }
  
  client.stop();
  
  // Open valve
  valve.open();
  
  // Start reading flow meter
  volume_mL = flowMeter.readVolume_mL(maxVolume_mL);

  Serial.print("Poured: ");
  Serial.print(volume_mL);
  Serial.println(" mL");
  
  success();
  
  // Close valve
  valve.close();

  // Don't send 0 volume
  if (volume_mL == 0)
  {
    fail("Poured zero volume.");
    return;
  }
  
  // Send pour results
  if (!client.connect(SETTINGS_SERVER_IP, SETTINGS_SERVER_PORT))
  {
    fail("Could not connect to server...");
    return;
  }
  
  if (!client.sendPourResult(tagData, volume_mL))
  {
    fail("Could not send pour result (send)");
    return;
  }
  
  if (!client.getPourResultResponse())
  {
    fail("Could not send pour result (recv)");
    return;
  }
  
  client.stop();
}
