// See LICENSE.txt for license details.

#ifndef POURLOGIC_HEX_STRING_H
#define POURLOGIC_HEX_STRING_H

#include <Arduino.h>
#include <String.h>

/*! \file HexString.h
 * \brief Functions for converting byte arrays to and from hexidecimal strings.
 */

/*! \brief Represent a byte array as a String in hexidecimal.
 */
String bytesToHexString(const byte* bytes, int length);

void bytesToHexString(char* result_string, const byte* bytes, int length);


/*! \brief Convert a hexidecimal String into an existing byte array.
 */
boolean hexStringToBytes(String const &hexString, byte *buffer, int &resultLength, int maxLength);

#endif // #ifndef POURLOGIC_HEX_STRING_H
