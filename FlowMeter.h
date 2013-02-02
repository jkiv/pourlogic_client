// See LICENSE.txt for license details.

#ifndef POURLOGIC_FLOW_METER_H
#define POURLOGIC_FLOW_METER_H

#include <Arduino.h>
#include "config.h"

/*! \brief Manages a flow meter who pulses on a digital input pin during flow.
 *  This class provides an interface to an interupt-based flow meter where the
 *  frequency of the pulses can be converted to a volumetric flow rate. This
 *  volumetric flow rate is then integrated to provide a volume.
 *
 *  Written for the G1/2 flow sensor from seeedstudio.com (POW110D3B) but
 *  should work with other interrupt-based flow meters.
 *
 *  *** 
 *
 *  We use a flow meter for determining the total volume that flowed
 *  during an arbitrary interval. To determine the volumetric flow rate,
 *  numerical mehods must be employed. However, for the total volume,
 *  the math involved is much simpler. The total volume is proportional to
 *  the number of pulses counted. See #calibrate for more details.
 *
 *  Calibrating your flow meter is recommended. Do this by allowing
 *  liquid to flow through the flow meter. Collect this liquid for
 *  measurement. If the number of pulses for this volume is known
 *  then the conversion constant is computed by the following formula:
 *
 *                           k = V / C
 * 
 *  You can run this calibration a number of times to find the average
 *  value (if you're feeling so keen).
 *
 *  In the future, calibration information can be set on the server to
 *  avoid reprogramming the device, perhaps being returned along with
 *  the pour request information (max volume, etc.)
 *
 *  \see #calibrate
 *  \see http://www.seeedstudio.com/depot/g12-water-flow-sensor-p-635.html
 */
class FlowMeter {

  private:
    unsigned short _pulse_count; //!< Accumulates when flow meter pulses on #_interruptPin when interrupts are attached and enabled
    int _interrupt_pin; //!< The input pin attached to the flow meter's output
    int _interrupt_number; //!< The interrupt number used for the flow meter
    
    void _startReading(); //!< Attach #_interruptPin to #pulse() using #_interruptNumber and set current flow meter to this instance
    void _stopReading(); //!< Detach interrupts and clear the current flow meter reference
    
  public:
    FlowMeter(){ /**/ }
    ~FlowMeter(){ /**/ }
    
    //!< Sets up the flow meter pins, etc.
    void begin(int interrupt_pin, int interrupt_number);
    
    //!< Read flowed volume in mL until a maximum volume is reached, a given time since the meter read flow has passed, and/or a total time has passed
    float readVolume_mL(int max_volume_mL, unsigned long last_pulse_timeout_ms = 2000, unsigned long total_timeout_ms = 30000, unsigned long delay_ms = 250);

    //!< Will run until targetPulseCount is reached; the measured volume should give volume per pulse for a known targetPulseCount.
    unsigned long calibrate(unsigned short target_pulse_count = 200, unsigned long last_pulse_timeout_ms = 2000, unsigned long total_timeout_ms = 30000, unsigned long delay_ms = 250);
    
    //!< Convert volume (in mL) to pulses
    unsigned short volumeToPulseCount(float volume);
    
    //!< Convert # of pulses to volume (in mL)
    float pulseCountToVolume(unsigned short count);
  
    //!< Interrupt handler for a flow meter pulse.
    static void pulse();
};

#endif

