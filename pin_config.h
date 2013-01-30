// See LICENSE.txt for license details.

#ifndef POURLOGIC_PIN_CONFIG_H
#define POURLOGIC_PIN_CONFIG_H

// Hardware/Board configuration (Arduino Uno)
#define FLOW1_PIN 2 //!< Flow meter (1) interrupt pin
#define FLOW2_PIN 3 //!< Flow meter (2) interrupt pin
#define RFID_ENABLE_PIN 5 //!< RFID reader ~enable pin
#define VALVE1_PIN 6 //!< Valve (1) open/close pin
#define VALVE2_PIN 7 //!< Valve (2) open/close pin
#define SD_REQUIRED_PIN 10 //< SD card required pin
#define SD_CS_PIN 4 //!< SD card CS pin
#define PIEZO_PIN 8 //!< Piezo element output pin (requires PWM)

// Pin interrupts
#define FLOW1_INTERRUPT 0
#define FLOW2_INTERRUPT 1

#endif // #ifndef POURLOGIC_PIN_CONFIG_H
