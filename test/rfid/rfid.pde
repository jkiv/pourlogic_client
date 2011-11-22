#if defined(ARDUINO) && ARDUINO > 18
  #include <SPI.h>
#endif

#include "config.h"
#include "ParallaxRFID.h"

#include <String.h>

static ParallaxRFID RFIDReader;

void setup()
{
    // Start serial communications
    Serial.begin(ParallaxRFID::RFID_BAUD);

    // Initialize RFID
    RFIDReader.begin(RFID1_ENABLE_PIN);
}

void loop()
{
    String tag = "";

    tag = RFIDReader.readRFID();

    Serial.println(tag);
}
