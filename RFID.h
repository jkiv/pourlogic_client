// See LICENSE.txt for license details.

#ifndef POURLOGIC_RFID_H 
#define POURLOGIC_RFID_H

#include <Arduino.h>
#include <String.h>

/*!
 * This RFID reader provides a 10-digit hexidecimal token represented
 * in ASCII.  The reader communicates over a 2400-baud serial connection
 * (RFID_EM41000#RFID_BAUD). The start of the tag data is represented by
 * the byte 0x0A (RFID_EM41000#RFID_START) and the end by 0x0D
 * (RFID_EM41000#RFID_END).
 *
 * The RFID reader is enabled (RFID_EM41000#enableRFID) by lowering the
 * digital pin connected to
 * \\ENABLE on the RFID board (RFID_EM41000#setPinEnable). Is is disabled
 * by setting the pin to high (RFID_EM41000#disableRFID).
 *
 * \brief A set of convenience functions for reading from a RFID_EM41000 reader.
 */
class RFID_EM41000 {
  public:
    RFID_EM41000(){}
    ~RFID_EM41000(){}
    // Constants 
    static const char RFID_START = 0x0A; //!< Byte representing the start of the tag data
    static const char RFID_END = 0x0D; //!< Byte representing the end of the tag data
    static const int RFID_LENGTH = 10; //!< Length of the RFID tag (16^10 = 1.099511627776E+12 unique IDs)
    static const long RFID_BAUD = 2400; //!< Baud rate of RFID communication
  
    boolean readRFID(String& rfid_result, unsigned long timeout_ms = 0); //!< Reads an RFID tag
    void enableRFID(); //!< Enables the RFID reader
    void disableRFID(); //!< Disables the RFID reader
    void begin(int pinEnable); //!< Sets the pin mode of the \\ENABLE pin (RFID_EM41000#setPinEnable) as OUTPUT and disables the RFID reader
  
  private:
    int _pinEnable; //!< Digital pin connected to /ENABLE
};

#endif // #ifndef POURLOGIC_RFID_H
