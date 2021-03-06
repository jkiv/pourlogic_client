// See LICENSE.txt for license details.

#ifndef POURLOGIC_VALVE_H
#define POURLOGIC_VALVE_H

#define VALVE_KEEP_CLOSED

#include <Arduino.h>

/*! \brief Opens (and/or closes) a solenoid valve.
 */
class Valve {
  private:
    int _valve_pin; //!< The digital pin which controls the valve
    boolean _normally_closed; //!< Whether HIGH is used to open the valve (true) or LOW (false).
  public:
    Valve(int valve_pin, boolean normally_closed = true);
    ~Valve();
    
    void open(); //!< Opens the valve
    void close(); //!< Closes the valve
};

#endif // #ifndef POURLOGIC_VALVE_H
