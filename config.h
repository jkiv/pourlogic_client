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

// Software configuration
// .. client info
#define SETTINGS_CLIENT_ID 0         //!< Your bot ID
#define SETTINGS_CLIENT_KEY "secret" //!< Keep this a secret

// .. flow meter tunables
#define SETTINGS_FLOW_PULSES_TO_ML 2.16 //!< This depends on your meter and should be determined experimentally based on your setup

// .. ethernet
#define SETTINGS_ETHERNET_MAC {0x00, 0x00, 0x00, 0x00, 0x00, 0x00} //!< Device's MAC address

#define SETTINGS_ETHERNET_USE_DHCP                  //!< Comment-out to manually specify IP, gateway, and subnet for the device.
#define SETTINGS_ETHERNET_GATEWAY {10, 0, 0, 1}     //!< Device's gateway (if not using DHCP)
#define SETTINGS_ETHERNET_SUBNET {255, 255, 255, 0} //!< Device's subnet (if not using DHCP)
#define SETTINGS_ETHERNET_IP IPAddress(10, 0, 0, 2) //!< Device's IP address (if not using DHCP)

// .. server info
#define SETTINGS_SERVER_IP IPAddress(10, 0, 0, 1)
#define SETTINGS_SERVER_PORT 80                          //!< HTTP port
#define SETTINGS_SERVER_HOSTNAME "pourlogic.com"         //!< HTTP hostname
#define SETTINGS_SERVER_POUR_REQUEST_URI "/pours/new/"   //!< URI to request when requesting to pour
#define SETTINGS_SERVER_POUR_RESULT_URI "/pours/create/" //!< URI to request when sending result


#endif // #ifndef POURLOGIC_CLIENT_CONFIG_H
