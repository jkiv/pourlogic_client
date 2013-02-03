// See LICENSE.txt for license details.

#include "RFID.h"
#include "StreamUtil.h"

RFID_EM41000::RFID_EM41000(Stream& rfid_serial, int enable_pin)
  : _enable_pin(enable_pin), _rfid_serial(rfid_serial)
{
  // Assume rfid_serial.begin() is called elsewhere for now
  //rfid_serial.begin(RFID_BAUD_RATE);
  
  // Set enable-pin pin-mode
  pinMode(enable_pin, OUTPUT);
  disableRFID(); 
}

boolean RFID_EM41000::readRFID(String& rfid_result, unsigned long timeout_ms) { 
  unsigned long start_time = millis();
  int bytes_read = -1;
  char tag_byte = '\0';
  char tag_data[RFID_LENGTH+1];
  tag_data[RFID_LENGTH] = '\0';

  // Flush serial buffer before turning on RFID transponder
  readUntilUnavailable(_rfid_serial);

  // Enable the RFID reader
  enableRFID();
  
  while(bytes_read <= RFID_LENGTH
        && (timeout_ms == 0 || millis() - start_time < timeout_ms))
  { 
    // Read the byte from the RFID reader
    tag_byte = _rfid_serial.read();
    
    // Are we currently populating tag_data?
    if (bytes_read >= 0) {
      
      // Should we stop populating tag_data?
      if(bytes_read >= RFID_LENGTH || tag_byte == RFID_START || tag_byte == RFID_END) {
        disableRFID();
        break; // stop reading
      }
    
      // Append the tag_data (weird bytes appear in >=Arduino 1.0, so we check byte content)
      if ((tag_byte >= 'A' && tag_byte <= 'F') || (tag_byte >= '0' && tag_byte <= '9')) { 
        tag_data[bytes_read] = tag_byte;
        bytes_read++;
      }
    }
    else if(tag_byte == RFID_START) {
      // We read the first byte of a key... signals that we start reading into tagData[]
      bytes_read = 0; 
    }
  }
  
  // Disable RFID (just to be safe)
  disableRFID();
  
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
  digitalWrite(_enable_pin, LOW);
}

void RFID_EM41000::disableRFID()
{
  digitalWrite(_enable_pin, HIGH);
}
