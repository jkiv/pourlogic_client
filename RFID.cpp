// See LICENSE.txt for license details.

#include "RFID.h"

boolean RFID_EM41000::readRFID(String& rfid_result, unsigned long timeout_ms) { 
  unsigned long start_time = millis();
  int bytes_read = -1;
  char tag_byte = '\0';
  char tag_data[RFID_LENGTH+1];
  tag_data[RFID_LENGTH] = '\0';

  // Enable the RFID reader
  enableRFID();
  
  while(bytes_read <= RFID_LENGTH
        && (millis() - start_time < timeout_ms || timeout_ms == 0))
  {
    
    // Disable the RFID reader as soon as possible
    // to prevent phantom reads...
    // FIXME phantom reads are a problem still (v1.0.1)
    if (Serial.available() + bytes_read >= RFID_LENGTH) {
      disableRFID();
    }
    
    if(Serial.available() > 0) {
      // Read the byte from the RFID reader
      tag_byte = Serial.read();
      
      if(tag_byte == RFID_END) {
        break; // stop reading
      }
        
      if (bytes_read >= 0) {  // (i.e. we're currently populating the tag_data)
        // Append the RFID data
        tag_data[bytes_read] = tag_byte;
        bytes_read++;
      }
      else if(tag_byte == RFID_START) { // we got the "start" byte of the data
        bytes_read = 0; // signals that we start reading into tag_data
      }
    }
  }
  
  // Disable RFID (just to be safe)
  disableRFID();
  
  // Avoid phantom reads
  // FIXME phantom reads are a problem still (v1.0.1)
  while(Serial.available())
    Serial.read();
  
  // Return result
  if (bytes_read == RFID_LENGTH) {
    rfid_result = tag_data;
    return true;
  }
  else {
    return false;
  }
  
}

void RFID_EM41000::enableRFID()
{
  digitalWrite(_pin_enable, LOW);
}

void RFID_EM41000::disableRFID()
{
  digitalWrite(_pin_enable, HIGH);
}

void RFID_EM41000::begin(int pin_enable)
{
  _pin_enable = pin_enable;
  pinMode(pin_enable,OUTPUT);
  disableRFID();
}
