// See LICENSE.txt for license details.

#include "Valve.h"

void Valve::begin(int valve_pin, boolean normally_closed)
{
  // Set whether or not HIGH is used to open the valve
  //       | N.C. | N.O.
  // ------+------+------
  // OPEN  | HIGH | LOW 
  // ------+------+------
  // CLOSE | LOW  | HIGH
  //
  // (Makes sense.)
  
  _normally_closed = normally_closed;
  
  // Set the output pins for controlling the valve
  _valve_pin = valve_pin;
  pinMode(valve_pin, OUTPUT);
  
  // Make sure the valve is closed by default
  close();
}

void Valve::open()
{
  digitalWrite(_valve_pin, _normally_closed ? HIGH : LOW);
}

void Valve::close()
{
  digitalWrite(_valve_pin, _normally_closed ? LOW : HIGH);
}
  
Valve::Valve()
{
  _normally_closed = true; // Use HIGH to open valve by default
  _valve_pin = -1; // Default to -1 so we know if it's been set
}

Valve::~Valve()
{
  // Make sure the valve is closed on destruction
  if (_valve_pin >= 0)
    close();
}
