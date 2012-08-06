// See LICENSE.txt for license details.

#ifndef POURLOGIC_OTP_H
#define POURLOGIC_OTP_H

#include <Arduino.h>
#include <EEPROM.h>

/*! \brief A persistent counter that uses EEPROM (5 bytes)
 *  \future moving keys
 */
class OTP {
  public:
    OTP() {}
    ~OTP() {}

    void begin(int baseOffset = 0); //!< Call to initialize the nonce generator
    uint32_t count();               //!< Returns the current valve of the counter
    void increment();               //!< Increment the counter
    void unset();                   //!< Mark the counter as uninitialized
    void set(uint32_t newCount);    //!< Set a new counter value
    void reload();                  //!< Load the counter from EEPROM
    
  private:
    static const int COUNTER_OFFSET = 1;
    static const int INITIALIZED_OFFSET = 0;
    static const byte INITIALIZED = 0xAA; //!< Arbitrary byte pattern to check if the counter has ever been initialized
  
    int _baseOffset;
    uint32_t _count;
};

#endif // #ifndef POURLOGIC_OTP_H
