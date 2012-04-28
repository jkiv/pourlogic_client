// See LICENSE.txt for license details.

#ifndef POURLOGIC_CLIENT_CONFIG_H
#define POURLOGIC_CLIENT_CONFIG_H

/*! \file config.h
 * \brief Configuration file for PourLogic setup
 * Values here can be edited but the software
 * must be recompiled and flashed onto the device.
 * Ideally there would be SD-card support and
 * a configuration would be parsed from a file.
 * However, attempts to do this cost 10k of
 * flash memory.
 */

// Hardware/Board configuration (Arduino Uno)
#define FLOW1_PIN 2 //!< Flow meter (1) interrupt pin
#define FLOW1_INTERRUPT 0
#define FLOW2_PIN 3 //!< Flow meter (2) interrupt pin
#define FLOW2_INTERRUPT 1
#define RFID_ENABLE_PIN 5 //!< RFID reader enable pin
#define VALVE1_PIN 6 //!< Valve (1) open/close pin
#define VALVE2_PIN 7 //!< Valve (2) open/close pin
#define SD_REQUIRED_PIN 10 //< SD card required pin
#define SD_CS_PIN 4 //!< SD card CS pin
#define PIEZO_PIN 8 //!< Piezo element output pin (requires PWM)

// Software configuration
// .. client info
#define SETTINGS_CLIENT_ID 0
#define SETTINGS_CLIENT_KEY "secret" //!< Keep this a secret
#define SETTINGS_ETHERNET_MAC {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
#define SETTINGS_ETHERNET_IP {10, 0, 0, 2}
#define SETTINGS_ETHERNET_GATEWAY {10, 0, 0, 1}
#define SETTINGS_ETHERNET_SUBNET {255, 255, 255, 0}

// .. server info
#define SETTINGS_SERVER_IP IPAddress(10, 0, 0, 1)
#define SETTINGS_SERVER_PORT 80
#define SETTINGS_SERVER_HOSTNAME "pourlogic.com" //!< HTTP hostname
#define SETTINGS_SERVER_REQUEST_URI "/pours/new/" //!< URI to request when requesting to pour
#define SETTINGS_SERVER_RESULT_URI "/pours/create/" //!< URI to request when sending result

// .. flow meter tunables
#define SETTINGS_FLOW_PULSES_TO_ML 2.16 //!< This depends on your meter and should be determined experimentally based on your setup

#endif
