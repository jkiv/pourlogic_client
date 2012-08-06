// See LICENSE.txt for license details.

#include "FlowMeter.h"

/**
 * Active flow meter instance (only one FlowMeter active at a time)
 * \todo Support for multiple flow meters
 */
static FlowMeter *activeFlowMeter = NULL;

/**
 * Flow meter interrupt handler. It increments the pulse count on the #activeFlowMeter.
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
  activeFlowMeter = this;
  attachInterrupt(_interruptNumber, FlowMeter::pulse, RISING);
  
  _pulseCount = 0;
}

/**
 * Detaches the active flow meter and its interrupts.
 */
void FlowMeter::_stopReading() {
  if (activeFlowMeter == this) {
    activeFlowMeter = NULL;
  }
  
  detachInterrupt(_interruptNumber);
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
  int maxVolume_pulses = (int)(((float) maxVolume_mL) / SETTINGS_FLOW_PULSES_TO_ML); //!< We convert the volume to a number of pulses to avoid floating point arithmetic.
  unsigned int pulseCountHistory[2] = {0, 0}; //!< The current (0) and previous (1) pulse counts for checking terminating conditions

  unsigned long startTime_ms = millis(); //!< The total timeout reference
  unsigned long timeSinceLastPulse_ms = millis(); //!< The end-of-pour timeout reference
 
  // Disable interrupts
  cli();

  // Attach interrupt on rising flow meter pin
  _startReading();

  // Capture pulses and think...
  for(;;) {
	  
    // Quickly grab a non-volitile version of _pulseCount
    cli();
    pulseCountHistory[0] = _pulseCount;
    sei();

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
      //Serial.println("Maximum total time elapsed.");
      break;
    }

    // Shift pulse history back
    pulseCountHistory[1] = pulseCountHistory[0];

    // Wait for a bit
    delay(delay_ms);
  }
  
  // Disable interrupts
  cli();

  // Detach flow meter interrupt
  _stopReading();
  
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
 * It can be shown that the volumetric flow rate, Q, is linearly
 * proportional to the pulse frequency, f, of the flow meter:
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
 * during the flow.
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
unsigned long FlowMeter::calibrate(unsigned long pourtime_ms, unsigned long timeout_ms) {
	
  unsigned long timeoutStart_ms = millis();
  unsigned long pulseCount = 0;
  
  Serial.println("Begin calibration...");
 
  // Attach interrupt on rising flow meter pin
  cli();
  _startReading();
 
  // Wait for flow to start
  while (_pulseCount == 0) {
	  
    // Enable interrupts
    sei();

    // Check for timeout
    if ((millis() - timeoutStart_ms) > timeout_ms) {
      Serial.println("Timeout while waiting for pulse.");
      cli();
      _stopReading();
      return pulseCount;
    }

    // Allow samples to come in
    delay(10);

    // Disable interrupts while we check _pulseCount
    cli();
  }

  // Enable interrupts
  sei();
 
  // Give user some time to pour
  delay(pourtime_ms);
  
  // Disable interrupts
  cli();
  
  // Get non-volitile version of _pulseCount
  pulseCount = _pulseCount;

  // Detach flow meter interrupt
  _stopReading();
  
  return pulseCount;
}
