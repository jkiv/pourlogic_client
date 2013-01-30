// See LICENSE.txt for license details.

#include "RFID.h"

boolean RFID_EM41000::readRFID(String& rfid_result, unsigned long timeout_ms) { 
  unsigned long startTime = millis();
  int bytesRead = -1;
  char tagByte = '\0';
  char tagData[RFID_LENGTH+1];
  tagData[RFID_LENGTH] = '\0';

  // Enable the RFID reader
  enableRFID();
  
  while(bytesRead <= RFID_LENGTH
        && (millis() - startTime < timeout_ms || timeout_ms == 0))
  {
    
    // Disable the RFID reader as soon as possible
    // to prevent phantom reads...
    if (Serial.available() + bytesRead >= RFID_LENGTH) {
      disableRFID();
    }
    
    if(Serial.available() > 0) {
      // Read the byte from the RFID reader
      tagByte = Serial.read();
      
      if(tagByte == RFID_END) {
        break; // stop reading
      }
        
      if (bytesRead >= 0) {  // (i.e. we're currently populating the tagData)
        // Append the RFID data
        tagData[bytesRead] = tagByte;
        bytesRead++;
      }
      else if(tagByte == RFID_START) { // we got the "start" byte of the data
        bytesRead = 0; // signals that we start reading into tagData
      }
    }
  }
  
  // Disable RFID (just to be safe)
  disableRFID();
  
  // Avoid phantom reads
  while(Serial.available())
    Serial.read();
  
  // Return result
  if (bytesRead == RFID_LENGTH) {
    rfid_result = tagData;
    return true;
  }
  else {
    return false;
  }
  
}

void RFID_EM41000::enableRFID()
{
  digitalWrite(_pinEnable, LOW);
}

void RFID_EM41000::disableRFID()
{
  digitalWrite(_pinEnable, HIGH);
}

void RFID_EM41000::begin(int pinEnable)
{
  _pinEnable = pinEnable;
  pinMode(pinEnable,OUTPUT);
  disableRFID();
}
