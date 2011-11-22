// See LICENSE.txt for license details.

#include "OTP.h"

void OTP::begin(int baseOffset)
{
  uint8_t *counterBytes = (uint8_t*) &_count;
    
  // Set the base offset in EEPROM
  _baseOffset = baseOffset;
    
  // Check whether the counter has ever been initialized
  if (EEPROM.read(_baseOffset + INITIALIZED_OFFSET) == INITIALIZED) // Initialized
  {
    reload();
  }
  else // Not initialized
  {
    set(0);
    EEPROM.write(_baseOffset + INITIALIZED_OFFSET, INITIALIZED);
  }
}

uint32_t OTP::count()
{
  return _count;
}

void OTP::increment()
{
  set(_count+1);
}

void OTP::reload()
{
  uint8_t *counterBytes = (uint8_t*) &_count;
  
  for (int i = 0; i < sizeof(_count); i++)
  {
    counterBytes[i] = EEPROM.read(_baseOffset + COUNTER_OFFSET + i);
  }
}

void OTP::set(uint32_t newCount)
{
  uint8_t *counterBytes = (uint8_t*) &_count;
  _count = newCount;
  
  for (int i = 0; i < sizeof(_count); i++)
  {
    // Only write if it has changed
    if (EEPROM.read(_baseOffset + COUNTER_OFFSET + i) != counterBytes[i])
    {
      EEPROM.write(_baseOffset + COUNTER_OFFSET + i, counterBytes[i]);
    }
  }
}

void OTP::unset()
{
  EEPROM.write(_baseOffset + INITIALIZED_OFFSET, 255);
}
