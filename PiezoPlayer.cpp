// See LICENSE.txt for license details.

#include "PiezoPlayer.h"

void PiezoPlayer::begin(int pin)
{
  _pin = pin;
  pinMode(_pin, OUTPUT);
}

void PiezoPlayer::play_melody(const uint16_t *melody, const uint8_t *beats, unsigned long length, unsigned long tempo_mspb)
{ 
  for (unsigned long i = 0; i < length; i++)
  {
    if (melody[i] > 0)
    {
      tone(_pin, melody[i], tempo_mspb * beats[i]); // asynchronous?
      delay(tempo_mspb * beats[i]);
    }
    else
    {
      noTone(_pin);
      delay(tempo_mspb * beats[i]);
    }
  }
  
  noTone(_pin);
}

