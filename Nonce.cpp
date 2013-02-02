// See LICENSE.txt for license details.

#include "Nonce.h"

void Nonce::begin(int base_offset) {
  // Set the base offset in EEPROM
  _base_offset = base_offset;
    
  // Check whether the counter has ever been initialized
  if (EEPROM.read(_base_offset + INITIALIZED_OFFSET) == INITIALIZED) { // Initialized 
    reload();
  }
  else { // Not initialized
    EEPROM.write(_base_offset + INITIALIZED_OFFSET, INITIALIZED);
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
  uint8_t *counter_bytes = (uint8_t*) &_count;
  
  for (int i = 0; i < sizeof(_count); i++) {
    counter_bytes[i] = EEPROM.read(_base_offset + COUNTER_OFFSET + i);
  }
}

void Nonce::set(uint32_t new_count) {
  uint8_t *counter_bytes = (uint8_t*) &_count;
  _count = new_count;
  
  for (int i = 0; i < sizeof(_count); i++) {
    // Only write if it has changed
    if (EEPROM.read(_base_offset + COUNTER_OFFSET + i) != counter_bytes[i]) {
      EEPROM.write(_base_offset + COUNTER_OFFSET + i, counter_bytes[i]);
    }
  }
}

void Nonce::unset() {
  EEPROM.write(_base_offset + INITIALIZED_OFFSET, 255);
}
