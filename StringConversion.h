// See LICENSE.txt for license details.

#ifndef STRING_CONVERSION_H
#define STRING_CONVERSION_H

#include <WProgram.h>
#include <String.h>

//!< Parse a String into a signed 32-bit integer
boolean stringToSigned(String const &s, int32_t &res);

//!< Parse a String into an unsigned 32-bit integer
boolean stringToUnsigned(String const &s, uint32_t &res);

#endif
