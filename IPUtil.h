// See LICENSE.txt for license details.

#ifndef KEGBOT_IP_UTIL_H
#define KEGBOT_IP_UTIL_H

/*! \file IPUtil.h
 * \brief IP utility structures and functions.
 */
 
#include <Arduino.h>
//#include <WProgram.h>

//#include <SPI.h>
//#include <Ethernet.h>
//#include <Client.h>
//#include <Server.h>
//#include <Udp.h>

#include "StringConversion.h"

/*! \brief An IPv4 address (4 bytes)
 */
typedef uint8_t ip4[4];

/*! \brief Duplicates information in `srcAddress' into `destAddress'.
 */
void ip4_copy(ip4 dst, const ip4 src);

/*! \brief Returns an ip4 as string in dotted quad format; e.g. "192.168.0.100".
 */
String ip4_to_str(const ip4 address);

//!< Converts a string in dotted quad format and saves it in the provided ip4 reference
boolean str_to_ip4(ip4 &address, String const &s);

#endif // #ifndef KEGBOT_IP_UTIL_H
