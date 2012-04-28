// See LICENSE.txt for license details.

#ifndef VALVE_H
#define VALVE_H

#include <Arduino.h>
//#include <WProgram.h>

/*! \brief Opens (and/or closes) a solenoid valve.
 */
class Valve
{
  private:
    int _valvePin; //!< The digital pin which controls the valve
    boolean _normallyClosed; //!< Whether HIGH is used to open the valve (true) or LOW (false).
  public:
    Valve();
    ~Valve();
    void begin(int valvePin, boolean normallyClosed = true); //!< Sets the valve pin and initializes the valve
    void open(); //!< Opens the valve
    void close(); //!< Closes the valve
};

#endif
