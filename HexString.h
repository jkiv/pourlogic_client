// See LICENSE.txt for license details.

#ifndef KEGBOT_HEX_STRING_H
#define KEGBOT_HEX_STRING_H

#include <Arduino.h>
//#include <WProgram.h>
#include <String.h>

/*! \file HexString.h
 * \brief Functions for converting byte arrays to and from hexidecimal strings.
 */

/*! \brief Represent a byte array as a String in hexidecimal.
 */
String bytesToHexString(const byte* bytes, int length);

/*! \brief Convert a hexidecimal String to a byte array.
 */
//byte* hexStringToBytes(String const &hexString, int &resultLength);

/*! \brief Convert a hexidecimal String into an existing byte array.
 */
boolean hexStringToBytes(String const &hexString, byte *buffer, int &resultLength, int maxLength);

#endif // #ifndef KEGBOT_HEX_STRING_H
