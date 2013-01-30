// See LICENSE.txt for license details.

#include "Nonce.h"

void Nonce::begin(int baseOffset) {
  // Set the base offset in EEPROM
  _baseOffset = baseOffset;
    
  // Check whether the counter has ever been initialized
  if (EEPROM.read(_baseOffset + INITIALIZED_OFFSET) == INITIALIZED) { // Initialized 
    reload();
  }
  else { // Not initialized
    EEPROM.write(_baseOffset + INITIALIZED_OFFSET, INITIALIZED);
    set(0);
  }
}

unsigned long Nonce::count() {
  return _count;
}

void Nonce::increment() {
  set(_count+1);
}

void Nonce::reload() {
  uint8_t *counterBytes = (uint8_t*) &_count;
  
  for (int i = 0; i < sizeof(_count); i++) {
    counterBytes[i] = EEPROM.read(_baseOffset + COUNTER_OFFSET + i);
  }
}

void Nonce::set(uint32_t newCount) {
  uint8_t *counterBytes = (uint8_t*) &_count;
  _count = newCount;
  
  for (int i = 0; i < sizeof(_count); i++) {
    // Only write if it has changed
    if (EEPROM.read(_baseOffset + COUNTER_OFFSET + i) != counterBytes[i]) {
      EEPROM.write(_baseOffset + COUNTER_OFFSET + i, counterBytes[i]);
    }
  }
}

void Nonce::unset() {
  EEPROM.write(_baseOffset + INITIALIZED_OFFSET, 255);
}
