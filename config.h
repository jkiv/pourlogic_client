// See LICENSE.txt for license details.

#ifndef POURLOGIC_CLIENT_CONFIG_H
#define POURLOGIC_CLIENT_CONFIG_H

/*! \file config.h
 * \brief Configuration file for PourLogic setup
 * Values here can be edited but the software
 * must be recompiled and flashed onto the device.
 */

// Client configuration
// .. client info
#define SETTINGS_CLIENT_ID 0         //!< Your bot ID
#define SETTINGS_CLIENT_KEY "secret" //!< Keep this a secret

// .. flow meter tunables
#define SETTINGS_FLOW_PULSES_TO_ML 2.16 //!< This depends on your meter and should be determined experimentally based on your setup (TODO from server)

// .. ethernet
#define SETTINGS_ETHERNET_USE_DHCP //!< Use DHCP for this device

#define SETTINGS_ETHERNET_MAC {0x00, 0x00, 0x00, 0x00, 0x00, 0x00} //!< Device's MAC address
#define SETTINGS_ETHERNET_IP  IPAddress(192, 168, 0, 100)          //!< Device's IP address (if not using DHCP)

// .. server info
#define SETTINGS_SERVER_IP   IPAddress(192, 168, 0, 101) //!< Server IP
#define SETTINGS_SERVER_PORT 80                          //!< Server port

#endif // #ifndef POURLOGIC_CLIENT_CONFIG_H
