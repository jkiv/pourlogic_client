// See LICENSE.txt for license details.

#include "FlowMeter.h"

/*! Active flow meter instance (only one FlowMeter active at a time)
 */
static FlowMeter *active_flow_meter = NULL;

/*! Flow meter interrupt handler. It increments the pulse count on the #activeFlowMeter.
 */
void FlowMeter::pulse() {
  if (active_flow_meter != NULL) {
    active_flow_meter->_pulse_count++;
  }
}

/**
 * Attaches the active flow meter and its interrupts. Resets #_pulse_count to 0.
 */
void FlowMeter::_startReading() {
  _pulse_count = 0;
  active_flow_meter = this;
  
  attachInterrupt(_interrupt_number, FlowMeter::pulse, RISING);
}

/**
 * Detaches the active flow meter and its interrupts.
 */
void FlowMeter::_stopReading() {
  detachInterrupt(_interrupt_number);
  active_flow_meter = NULL;
}

/**
 * Sets up the Arduino pins for the flow meter.
 * \param interruptPin The pin the flow meter iterrupts on.
 * \param interruptNumber The interupt number typically corresponds to the pin. Check your chip's documentation for the pin <=> interrupt number relation.
 */
FlowMeter::FlowMeter(int interrupt_pin, int interrupt_number)
  : _interrupt_pin(interrupt_pin), _interrupt_number(interrupt_number)
{
  // Set up pins
  pinMode(interrupt_pin, INPUT);
}

//!< Convert volume (in mL) to pulses
unsigned short FlowMeter::volumeToPulseCount(float volume) {
  return volume / SETTINGS_FLOW_PULSES_TO_ML;
}

//!< Convert # of pulses to volume (in mL)
float FlowMeter::pulseCountToVolume(unsigned short count) {
  return count * SETTINGS_FLOW_PULSES_TO_ML;
}

/**
 * Returns the poured volume in millileters.
 * \param maxVolume_mL The function will return after this many millilitres have been poured. If this is zero or negative then there is no limit.
 * \param last_pulse_timeout_ms The function will return after this many milliseconds since flow was last detected.
 * \param total_timeout_ms Causes the function to return after this many milliseconds since the function was called.
 * \param delay_ms The time to wait between checking any of the terminating conditions.
 */
float FlowMeter::readVolume_mL(int max_volume_mL, unsigned long last_pulse_timeout_ms, unsigned long total_timeout_ms, unsigned long delay_ms) {  
  unsigned short max_volume_pulses = volumeToPulseCount(max_volume_mL); //!< We convert the volume to a number of pulses to avoid floating point arithmetic.
  unsigned short pulse_count_history[2] = {0, 0}; //!< The current (0) and previous (1) pulse counts for checking terminating conditions
  unsigned long start_time_ms = millis(); //!< The total timeout reference
  unsigned long time_of_last_pulse_ms = start_time_ms; //!< The end-of-pour timeout reference
  
  // Attach interrupt on rising flow meter pin
  _startReading();

  // Capture pulses and think...
  for(;;) {
	  
    // Quickly grab and copy _pulse_count to a non-volitile variable
    noInterrupts();
    pulse_count_history[0] = _pulse_count;
    interrupts();

    // Remember moment of last detected pulse 
    if (pulse_count_history[0] > pulse_count_history[1]) {
      time_of_last_pulse_ms = millis();
    }
    
    // Have we reached the maximum volume?
    if (max_volume_pulses > 0 && pulse_count_history[0] >= max_volume_pulses) {
      //Serial.println("Maximum volume reached...");
      break;
    }

    // Time out if it has been too long since last detected flow
    if (millis() > time_of_last_pulse_ms + last_pulse_timeout_ms) {
      //Serial.println("Timeout since last detected flow...");
      break;
    }

    // Time out if we have been reading for too long 
    if (millis() >= start_time_ms + total_timeout_ms) {
      //Serial.println("Timeout since start of read...");
      break;
    }

    // Shift pulse history back
    pulse_count_history[1] = pulse_count_history[0];

    // Wait for a bit
    delay(delay_ms);
  }
  
  // Detach flow meter interrupt
  _stopReading();
  
  // Return the total poured volume
  return pulseCountToVolume(_pulse_count);
}

/**
 * Helps calibrate the flow meter.
 * 
 * \param pourtime_ms The time given for a pour to take place once it has started.
 * \param timeout_ms The time allowed for a pour to begin before returning.
 * \return The number of pulses during the pour.
 * 
 * \todo Mimick terminating conditions of #readVolume_mL (other than max. volume) but return pulse count rather than volume in mL.
 * 
 * Calibration requires measuring a resultant volume and number of ticks
 * from start of flow to end of flow. For example, pouring into a
 * measuring cup a few hundred millilitres will suffice. When flow
 * stoppage has been detected, the device will report the corresponding
 * number of pulses. These two values are used to calculate the
 * pulse-to-volume constant as described below.
 *
 * We assume that the volumetric flow rate, Q, is linearly
 * proportional (k) to the pulse frequency, f, of the flow meter:
 *
 *                             Q(t) = k*f(t)
 *
 * Integrating both sides with respect to time we get the following:
 * 
 *                                V = k*C
 *
 * where V is the total volume and C is the total number of pulses over
 * the flow interval. The initial volume and counts are zero. Note that
 * the total volume can be determined directly from the number of
 * pulses of the flow meter, regardless of how the flow rate changes
 * during the flow (within operating range of the flow meter, of course).
 * 
 * To calibrate the flow meter, pour out and measure a volume and use
 * the corresponding number of pulses to calculate the conversion factor.
 * For example, if a volume of 200 mL is poured and and a count of 98
 * pulses is read, the conversion constant is found in the following
 * manner:
 *                          k = 200 mL / 98 pulses
 *                          k = 2.040... mL/pulse
 *
 * This value should be determined upon first set up and need not be
 * changed afterward. If operating conditions do change, don't hesitate
 * to re-calibrate. The result should be similar.
 * 
 * Please note that by specifying #pourtime_ms will allow the average
 * flow rate to be calculated. Make sure this flow rate is within the
 * operating conditions of the flow meter.
 */
unsigned long FlowMeter::calibrate(unsigned short target_pulse_count, unsigned long last_pulse_timeout_ms, unsigned long total_timeout_ms, unsigned long delay_ms) {
  unsigned short pulse_count_history[2] = {0, 0}; //!< The current (0) and previous (1) pulse counts for checking terminating conditions
  unsigned long start_time_ms = millis(); //!< The total timeout reference
  unsigned long time_of_last_pulse_ms = start_time_ms; //!< The end-of-pour timeout reference (initially, sufficiently large)
  boolean success = false;
  
  // Attach interrupt on rising flow meter pin
  _startReading();

  // Capture pulses and think...
  for(;;) {
	  
    // Quickly grab and copy _pulse_count to a non-volitile variable
    noInterrupts();
    pulse_count_history[0] = _pulse_count;
    interrupts();

    // Remember moment of last detected pulse 
    if (pulse_count_history[0] > pulse_count_history[1]) {
      time_of_last_pulse_ms = millis();
    }
    
    // Have we reached the maximum volume?
    if (pulse_count_history[0] >= target_pulse_count) {
      //Serial.println("Target volume reached...");
      success = true;
      break;
    }

    // Time out if it has been too long since last detected flow
    if (millis() > time_of_last_pulse_ms + last_pulse_timeout_ms) {
      //Serial.println("Timeout since last detected flow...");
      break;
    }

    // Time out if we have been reading for too long 
    if (millis() >= start_time_ms + total_timeout_ms) {
      //Serial.println("Timeout since start of read...");
      break;
    }

    // Shift pulse history back
    pulse_count_history[1] = pulse_count_history[0];

    // Wait for a bit
    delay(delay_ms);
  }
  
  // Detach flow meter interrupt
  _stopReading();
  
  // Return the total poured volume
  return success;
}
