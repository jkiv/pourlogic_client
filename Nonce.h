// See LICENSE.txt for license details.

#ifndef POURLOGIC_NONCE_H
#define POURLOGIC_NONCE_H

#include <Arduino.h>
#include <EEPROM.h>

/*! \brief A persistent nonce generator that uses EEPROM (5 bytes)
 *  Simply, this is a persistent counter that increments.
 */
class Nonce {
  public:
    Nonce() {}
    ~Nonce() {}

    void begin(int baseOffset = 0); //!< Call to initialize the nonce generator
    //virtual uint32_t next();        //!< Returns a nonce and increments
    
    unsigned long count();                       //!< Returns the current valve of the counter
    void increment();                       //!< Increment the counter
    void unset();                           //!< Mark the counter as uninitialized
    void set(uint32_t newCount);            //!< Set a new counter value
    void reload();                          //!< Load the counter from EEPROM
    
  private:
    static const int COUNTER_OFFSET = 1;
    static const int INITIALIZED_OFFSET = 0;
    static const byte INITIALIZED = 0xAA; //!< Arbitrary byte pattern to check if the counter has ever been initialized
  
    int _baseOffset;
    unsigned long _count;
};

#endif // #ifndef POURLOGIC_NONCE_H
