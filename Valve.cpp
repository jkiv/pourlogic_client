// See LICENSE.txt for license details.

#include "Valve.h"

Valve::Valve(int valve_pin, boolean normally_closed)
  : _valve_pin(valve_pin), _normally_closed(normally_closed)
{
  // Set whether or not HIGH is used to open the valve
  //       | N.C. | N.O.
  // ------+------+------
  // OPEN  | HIGH | LOW 
  // ------+------+------
  // CLOSE | LOW  | HIGH
  //
  // (Makes sense.)
  
  // Set up pin modes
  pinMode(valve_pin, OUTPUT);
  
#ifdef VALVE_KEEP_CLOSED
  // Make sure the valve is kept closed
  close();
#endif
}

void Valve::open() {
  digitalWrite(_valve_pin, _normally_closed ? HIGH : LOW);
}

void Valve::close() {
  digitalWrite(_valve_pin, _normally_closed ? LOW : HIGH);
}

Valve::~Valve() {
#ifdef VALVE_KEEP_CLOSED
  // Make sure the valve is closed kept closed
  close();
#endif
}
