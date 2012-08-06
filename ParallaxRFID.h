// See LICENSE.txt for license details.

#ifndef POURLOGIC_PARALLAX_RFID_H 
#define POURLOGIC_PARALLAX_RFID_H

#include <Arduino.h>
#include <String.h>

/*!
 * This RFID reader provides a 10-digit hexidecimal token represented
 * in ASCII.  The reader communicates over a 2400-baud serial connection
 * (ParallaxRFID#RFID_BAUD). The start of the tag data is represented by
 * the byte 0x0A (ParallaxRFID#RFID_START) and the end by 0x0D
 * (ParallaxRFID#RFID_END).
 *
 * The RFID reader is enabled (ParallaxRFID#enableRFID) by lowering the
 * digital pin connected to
 * \\ENABLE on the RFID board (ParallaxRFID#setPinEnable). Is is disabled
 * by setting the pin to high (ParallaxRFID#disableRFID).
 *
 * \brief A set of convenience functions for reading from a Parallax RFID reader.
 */
class ParallaxRFID {
  public:
    ParallaxRFID(){}
    ~ParallaxRFID(){}
    // Constants 
    static const char RFID_START = 0x0A; //!< Byte representing the start of the tag data
    static const char RFID_END = 0x0D; //!< Byte representing the end of the tag data
    static const int RFID_LENGTH = 10; //!< Length of the RFID tag (16^10 = 1.099511627776E+12 unique IDs)
    static const long RFID_BAUD = 2400; //!< Baud rate of RFID communication
  
    boolean readRFID(String& rfid_result, unsigned long timeout_ms = 0); //!< Reads an RFID tag
    void enableRFID(); //!< Enables the RFID reader
    void disableRFID(); //!< Disables the RFID reader
    void begin(int pinEnable); //!< Sets the pin mode of the \\ENABLE pin (ParallaxRFID#setPinEnable) as OUTPUT and disables the RFID reader
  
  private:
    int _pinEnable; //!< Digital pin connected to /ENABLE
};

#endif // #ifndef POURLOGIC_PARALLAX_RFID_H
