// See LICENSE.txt for license details.

#include "Valve.h"

void Valve::begin(int valvePin, boolean normallyClosed)
{
  // Set whether or not HIGH is used to open the valve
  _normallyClosed = normallyClosed;
  
  // Set the output pins for controlling the valve
  _valvePin = valvePin;
  pinMode(valvePin, OUTPUT);
  
  // Make sure the valve is closed by default
  close();
}

void Valve::open()
{
  // Open the valve by setting the valve pin LOW
  if (_normallyClosed)
    digitalWrite(_valvePin, HIGH);
  else
    digitalWrite(_valvePin, LOW);
}

void Valve::close()
{
  // Close the valve by setting the valve pin HIGH
  if (_normallyClosed)
    digitalWrite(_valvePin, LOW);
  else
    digitalWrite(_valvePin, HIGH);
}
  
Valve::Valve()
{
  _normallyClosed = true; // Use HIGH to open valve by default
  _valvePin = -1; // Default to -1 so we know if it's been set
}

Valve::~Valve()
{
  // Make sure the valve is closed on destruction
  if (_valvePin >= 0)
    close();
}
