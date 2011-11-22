// See LICENSE.txt for license details.

#include "config.h"

// General
#include <WProgram.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

// SPI
#include <SPI.h>

// Ethernet
#include <Client.h>
#include <Ethernet.h>
#include <Server.h>
#include <Udp.h>

// EEPROM
#include <EEPROM.h>

// SD
//#include <SD.h> // TODO read settings from SD

// Strings
#include <String.h>

// SHA Hashing (https://github.com/Cathedrow/Cryptosuite/)
#include <sha256.h>

// PourLogic
#include "BasicOO.h"
#include "StringConversion.h"
#include "IPUtil.h"
#include "StreamUtil.h"
#include "HexString.h"
#include "ParallaxRFID.h"
#include "FlowMeter.h"
#include "Valve.h"
#include "PiezoPlayer.h"
#include "PourLogicClient.h"

static PiezoPlayer piezo;
static FlowMeter flowMeter;
static ParallaxRFID rfidReader;
static Valve valve;

uint16_t fail_melody[] = {NOTE_B2, NOTE_AS2, NOTE_B2, NOTE_AS2, NOTE_B2, NOTE_AS2, NOTE_B2, NOTE_AS2};
uint8_t fail_beats[] = {1, 1, 1, 1, 1, 1, 1, 1};
uint8_t fail_length = 8;

uint16_t success_melody[] =   {NOTE_DS3, 0, NOTE_DS3, 0, NOTE_DS3, 0, NOTE_DS3, NOTE_B2, NOTE_CS3, NOTE_DS3, 0, NOTE_CS3, NOTE_DS3};
uint8_t success_beats[] =  {1, 1, 1, 1, 1, 1, 6, 6, 6, 4, 1, 2, 9};
uint8_t success_length = 13;

//!< Called when failure occurs (i.e. often) 
void fail(const char* msg = 0)
{
  // Print fail msg
  if (msg)
    Serial.println(msg);

  // Play fail music
  piezo.play_melody(fail_melody, fail_beats, fail_length, 50); 
}

//!< Called when success occurs
void success()
{
  // Play success music
  piezo.play_melody(success_melody, success_beats, success_length, 50);
}

//!<Setup the PourLogic controller environment and settings
void setup()
{   
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
    uint8_t ethernet_mac[] = SETTINGS_ETHERNET_MAC;
    uint8_t ethernet_ip[] = SETTINGS_ETHERNET_IP;
    uint8_t ethernet_gateway[] = SETTINGS_ETHERNET_GATEWAY;
    uint8_t ethernet_subnet[] = SETTINGS_ETHERNET_SUBNET;
    
    Ethernet.begin(ethernet_mac,
                   ethernet_ip,
                   ethernet_gateway,
                   ethernet_subnet);
    
    // Set up piezo
    piezo.begin(PIEZO_PIN);
}

/*! \brief Handle pourlogic day to day operations.
 */
void loop()
{
  ip4 server_ip = SETTINGS_SERVER_IP;
  PourLogicClient client(server_ip, SETTINGS_SERVER_PORT);
  
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
  if (!client.connect())
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
  if (!client.connect())
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
