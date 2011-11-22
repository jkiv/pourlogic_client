// See LICENSE.txt for license details.

#include "ParallaxRFID.h"

String ParallaxRFID::readRFID(unsigned long timeout_ms)
{ 
  unsigned long startTime = millis();
  int bytesRead = -1;
  char tagByte = '\0';
  String tagData = "";

  // Enable the RFID reader
  enableRFID();
  
  while(bytesRead <= RFID_LENGTH && (millis() - startTime < timeout_ms || timeout_ms == 0))
  {
    Serial.print(bytesRead);
    Serial.print(" ");
    Serial.println(Serial.available());
   
    // Disable the RFID reader as soon as possible
    // to prevent phantom reads...
    if (Serial.available() + bytesRead >= RFID_LENGTH)
    {
      disableRFID();
    }
    
    if(Serial.available() > 0)
    {
      // Read the byte from the RFID reader
      tagByte = Serial.read();
      
      if (bytesRead >= 0) // (i.e. we're currently populating the tagData)
      {
        if(bytesRead >= RFID_LENGTH || tagByte == RFID_START || tagByte == RFID_END)
        {
          break; // stop reading
        }
        
        // Append the RFID data
        tagData.concat(tagByte);
        bytesRead++;
      }
      else if(tagByte == RFID_START) // we got the "start" byte of the data
      {
        bytesRead = 0; // signals that we start reading into tagData[]
      }
    }
  }
  
  // Disable RFID (just to be safe)
  disableRFID();
  
  // Avoid phantom reads FIXME
  do
  {
    while(Serial.available())
      Serial.read();
      
    delay(500);
  } while (Serial.available());
  
  // Return result
  if (bytesRead == RFID_LENGTH)
    return tagData;
  else
    return "";
}

void ParallaxRFID::enableRFID()
{
  digitalWrite(_pinEnable, LOW);
}

void ParallaxRFID::disableRFID()
{
  digitalWrite(_pinEnable, HIGH);
}

void ParallaxRFID::begin(int pinEnable)
{
  _pinEnable = pinEnable;
  pinMode(pinEnable,OUTPUT);
  disableRFID();
}
