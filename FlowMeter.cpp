// See LICENSE.txt for license details.

#include "FlowMeter.h"

static FlowMeter *activeFlowMeter = NULL; //!< Active flow meter instance (only one FlowMeter active at a time)

void FlowMeter::pulse()
{
  if (activeFlowMeter != NULL)
    activeFlowMeter->_pulseCount++;
}

void FlowMeter::_startReading()
{
  activeFlowMeter = this;
  attachInterrupt(_interruptNumber, FlowMeter::pulse, RISING);
  
  _pulseCount = 0;
}

void FlowMeter::_stopReading()
{
  activeFlowMeter = NULL;
  detachInterrupt(_interruptNumber);
}

void FlowMeter::begin(int interruptPin, int interruptNumber)
{
  _interruptPin = interruptPin;
  _interruptNumber = interruptNumber;
  
  // Set the pin mode for the interrupt pin
  pinMode(interruptPin, INPUT);
}

float FlowMeter::readVolume_mL(float maxVolume_mL, unsigned long lastPulseTimeout_ms, unsigned long totalTimeout_ms, unsigned long delay_ms)
{
  // ==== METER CALIBRATION ==================================================
  // From docs Q = kf, where Q is the flow rate in L/min, f is the pulse
  // frequency in Hz, and k is the conversion constant (7.5^-1 in our meter's
  // documenation).  However, 7.5^-1 doesn't work for our meter.
  //
  // NOTE: Q == L/min and f == P/s, meaning we can cancel out minutesand
  // seconds. This means the volume (V = Q*t) is proportional to the pulse
  // count as well.
  //
  // To calibrate our flow meter, we poured 200 mL and counted 98 pulses on
  // average over several tests:
  //
  //     k = 200 mL/98 pulses = 2.040... mL/pulse
  //
  // This 200 mL was over approximately 10 seconds:
  //
  //     Q = 200 mL / 10 s = 20 mL/s = 0.02 L/s = 1.2 L/min
  //
  // NOTE: 1.2 L/min is within the operating range of the meter.
  //
  // So, imagine if we get 98 pulses we can determine how much volume that is:
  //    V = 98 pulses * 2.040... mL/pulse = 200 mL
  //
  // And over 10 seconds:
  //     Q = 200 mL / 10 s = 20 mL/s = 0.02 L/s = 1.2 L/min
  
  float pulsesTomL =  SETTINGS_FLOW_PULSES_TO_ML; // Using calibrated meter value
 
  unsigned long startTime_ms = millis(); //!< Our timeout (total) reference
  unsigned long timeSinceLastPulse_ms = millis(); //!< Our timeout (total) reference
 
  float pulseCount_tmp = 0.0f;
  float totalVolume_mL = 0.0f;
    
  // Attach interrupt on rising flow meter pin
  _startReading();
  
  // Start counting
  sei();

  // Instead of missing pulses, we're going to try to capture pulses while we
  // are calculating the flow rate from the previously calculated volume.
  // If this doesn't work, simply assume that the number of pulses during
  // the logic is ~0. 
  for(;;)
  {
    delay(delay_ms);
    
    cli();
    pulseCount_tmp = _pulseCount;
    sei();
    
    // Remember now
    if (pulseCount_tmp > 0)
    {
      timeSinceLastPulse_ms = millis();
    }
    
    // Prevent maximum volume from being reached
    if (maxVolume_mL > 0 && (float) pulseCount_tmp * pulsesTomL >= maxVolume_mL)
    {
      Serial.println("Maximum volume reached...");
      break;
    }

    if (millis() - timeSinceLastPulse_ms > lastPulseTimeout_ms)
    {
      Serial.println("Timeout since last detected flow...");
      break;
    }
    
    // Remember when we last had pouring so we can time out
    if (pulseCount_tmp == 0)
    {
      timeSinceLastPulse_ms = millis();
    }

    // Time out if we have been reading for too long 
    if ((millis() - startTime_ms) >= totalTimeout_ms)
    {
      Serial.println("Maximum total time elapsed.");
      break;
    }
    
  }
  
  // Disable interrupts
  cli();

  // Detach flow meter interrupt
  _stopReading();
  
  return (float) _pulseCount * pulsesTomL;
}

void FlowMeter::calibrate(unsigned long pourtime_ms, unsigned long timeout_ms)
{
  boolean started = false;
  unsigned long startTime_ms = millis();
  
  Serial.println("Begin calibration...");
  
  // Zero count
  _pulseCount = 0;
  
  // Attach interrupt on rising flow meter pin
  _startReading();

  // Enable interrupts while we process other things
  sei();
  
  for(;;)
  {
    if (_pulseCount > 0)
    {
      startTime_ms = millis();
      Serial.println("Starting calibration.");
      break;
    }
    else if ((millis() - startTime_ms) > timeout_ms)
    {
      Serial.println("Timeout while waiting for pulse.");
      cli();
      _stopReading();
      return;
    }
    
    delay(10);
  }
  
  delay(pourtime_ms);
  
  // Disable interrupts
  cli();

  // Detach flow meter interrupt
  _stopReading();
  
  Serial.print(_pulseCount);
  Serial.print(" pulses over ");
  Serial.print(millis() - startTime_ms);
  Serial.println(" ms");
}
