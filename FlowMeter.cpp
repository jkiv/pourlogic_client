// See LICENSE.txt for license details.

#include "FlowMeter.h"

/*! Active flow meter instance (only one FlowMeter active at a time)
 */
static FlowMeter *activeFlowMeter = NULL;

/*! Flow meter interrupt handler. It increments the pulse count on the #activeFlowMeter.
 */
void FlowMeter::pulse() {
  if (activeFlowMeter != NULL) {
    activeFlowMeter->_pulseCount++;
  }
}

/**
 * Attaches the active flow meter and its interrupts. Resets #_pulseCount to 0.
 */
void FlowMeter::_startReading() {
  _pulseCount = 0;
  activeFlowMeter = this;
  
  attachInterrupt(_interruptNumber, FlowMeter::pulse, RISING);
}

/**
 * Detaches the active flow meter and its interrupts.
 */
void FlowMeter::_stopReading() {
  detachInterrupt(_interruptNumber);
  activeFlowMeter = NULL;
}

/**
 * Sets up the Arduino pins for the flow meter.
 * \param interruptPin The pin the flow meter iterrupts on.
 * \param interruptNumber The interupt number typically corresponds to the pin. Check your chip's documentation for the pin <=> interrupt number relation.
 */
void FlowMeter::begin(int interruptPin, int interruptNumber) {
  _interruptPin = interruptPin;
  _interruptNumber = interruptNumber;
  
  // Set the pin mode for the interrupt pin
  pinMode(interruptPin, INPUT);
}

/**
 * Returns the poured volume in millileters.
 * \param maxVolume_mL The function will return after this many millilitres have been poured. If this is zero or negative then there is no limit.
 * \param lastPulseTimeout_ms The function will return after this many milliseconds since flow was last detected.
 * \param totalTimeout_ms Causes the function to return after this many milliseconds since the function was called.
 * \param delay_ms The time to wait between checking any of the terminating conditions.
 */
float FlowMeter::readVolume_mL(int maxVolume_mL, unsigned long lastPulseTimeout_ms, unsigned long totalTimeout_ms, unsigned long delay_ms) {  
  unsigned long maxVolume_pulses = (unsigned long)(((float) maxVolume_mL) / SETTINGS_FLOW_PULSES_TO_ML); //!< We convert the volume to a number of pulses to avoid floating point arithmetic.
  unsigned short pulseCountHistory[2] = {0, 0}; //!< The current (0) and previous (1) pulse counts for checking terminating conditions

  unsigned long startTime_ms = millis(); //!< The total timeout reference
  unsigned long timeSinceLastPulse_ms = millis(); //!< The end-of-pour timeout reference
  
  // Attach interrupt on rising flow meter pin
  _startReading();

  // Capture pulses and think...
  for(;;) {
	  
    // Quickly grab and copy _pulseCount to a non-volitile variable
    noInterrupts();
    pulseCountHistory[0] = _pulseCount;
    interrupts();

    // Remember moment of last detected pulse 
    if (pulseCountHistory[0] > pulseCountHistory[1]) {
      timeSinceLastPulse_ms = millis();
    }
    
    // Have we reached the maximum volume?
    if (maxVolume_pulses > 0 && pulseCountHistory[0] >= maxVolume_pulses) {
      //Serial.println("Maximum volume reached...");
      break;
    }

    // Time out if it has been too long since last detected flow
    if (millis() > timeSinceLastPulse_ms + lastPulseTimeout_ms) {
      //Serial.println("Timeout since last detected flow...");
      break;
    }

    // Time out if we have been reading for too long 
    if (millis() >= startTime_ms + totalTimeout_ms) {
      //Serial.println("Timeout since start of read...");
      break;
    }

    // Shift pulse history back
    pulseCountHistory[1] = pulseCountHistory[0];

    // Wait for a bit
    delay(delay_ms);
  }
  
  // Detach flow meter interrupt
  _stopReading();
  
  Serial.println(_pulseCount);
  
  // Return the total poured volume
  return (float) _pulseCount * SETTINGS_FLOW_PULSES_TO_ML;
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
unsigned long FlowMeter::calibrate(unsigned short targetPulseCount, unsigned long pourtime_ms, unsigned long timeout_ms, unsigned long delay_ms) {
  unsigned short pulseCountHistory[2] = {0, 0}; //!< The current (0) and previous (1) pulse counts for checking terminating conditions
  unsigned long startTime_ms = millis(); //!< The total timeout reference
  unsigned long timeSinceLastPulse_ms = millis(); //!< The end-of-pour timeout reference
  boolean success = true;
  
  // Attach interrupt on rising flow meter pin
  _startReading();

  // Capture pulses and think...
  for(;;) {
	  
    // Quickly grab and copy _pulseCount to a non-volitile variable
    noInterrupts();
    pulseCountHistory[0] = _pulseCount;
    interrupts();

    // Remember moment of last detected pulse 
    if (pulseCountHistory[0] > pulseCountHistory[1]) {
      timeSinceLastPulse_ms = millis();
    }

    // Time out if it has been too long since last detected flow
    if (millis() > timeSinceLastPulse_ms + lastPulseTimeout_ms) {
      //Serial.println("Timeout since last detected flow...");
      break;
    }
    
    // Time out if we have been reading for too long 
    if (millis() >= startTime_ms + totalTimeout_ms) {
      //Serial.println("Timeout since start of read...");
      break;
    }

    // Have we reached the calibration pulse count?
    if (pulseCountHistory[0] >= targetPulseCount) {
      //Serial.println("Target pulse count reached");
      success = true;
      break;
    }

    // Shift pulse history back
    pulseCountHistory[1] = pulseCountHistory[0];

    // Wait for a bit
    delay(delay_ms);
  }
  
  // Detach flow meter interrupt
  _stopReading();
  
  // Return the total poured volume
  return success;
}
