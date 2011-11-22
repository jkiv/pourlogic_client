// See LICENSE.txt for license details.

#ifndef KEGBOT_STREAM_UTIL_H
#define KEGBOT_STREAM_UTIL_H

#include <SPI.h>

//#include "BasicOO.h"
#include "Stream.h"
#include "String.h"

/*! \file HTTPUtil.h
 * \brief Utility functions for working with HTTP.
 */

/*!
 * \brief Reads data from #stream into #body until #pattern is witnessed.  #body will contain #pattern.
 * \param stream The Stream to read from; it should be available().
 * \param pattern If this pattern is encountered, reading will stop and the function will return true.
 * \param body All data read will be stored in this string.
 * \param maximumBytes The maximum number of bytes to read into before failure
 * \return False will be returned if the operation stopped for any reason other than hitting the pattern.  If the pattern was seen, this function returns true.
 */
bool readStreamUntil(Stream *stream, String const &pattern, String &body, int maximumBytes = 0);

/*!
 * \brief Reads data from #stream until #pattern is witnessed.  All read data is discarded.
 * \param stream The Stream to read from; it should be available().
 * \param pattern If this pattern is encountered, reading will stop and the function will return true.
 * \returns False will be returned if the operation stopped for any reason other than hitting the pattern.  If the pattern was seen, this function returns true.
 */
bool readStreamUntil(Stream *stream, String const &pattern);

/*!
 * \brief Reads data from #stream until the character being read is not in #alphabet.
 * All read data is discarded except the first character that is encountered that is not in the alphabet, which is left as the next readable character.
 * \param stream The Stream to read from.
 * \param alphabet The stream is read until a character that is not in this string is encountered.
 * \returns False will be returned if no character outside of the #alphabet was encountered, true if a character was encountered that was not in the alphabet.
 */
bool readStreamWhileIn(Stream *stream, String const &alphabet);
bool readStreamWhileIn(Stream *stream, String const &alphabet, String &body, int maximumBytes = 0);

/*!
 * \brief Reads data from #stream until the character being read is in #alphabet.
 * All read data is discarded except the first character that is encountered that is not in the alphabet, which is left as the next readable character.
 * \param stream The Stream to read from.
 * \param alphabet The stream is read until a character that is not in this string is encountered.
 * \returns False will be returned if no character outside of the #alphabet was encountered, true if a character was encountered that was not in the alphabet.
 */
bool readStreamWhileNotIn(Stream *stream, String const &alphabet);
bool readStreamWhileNotIn(Stream *stream, String const &alphabet, String &body, int maximumBytes = 0);

//!< Generalized conditional stream reading function used for both #readStreamWhileIn and #readStreamWhileNotIn
bool _readStreamWhile(Stream *stream, String const &alphabet, bool whileInAlphabet);
bool _readStreamWhile(Stream *stream, String const &alphabet, bool whileInAlphabet, String &body, int maximumBytes = 0);

/*!
 * \brief Waits for data to be ready to read, or times out
 * \param timeout_ms Timeout in milliseconds (default = 2000)
 * \returns True on ready, false on timeout
 */
bool readyOrTimeout(Stream *stream, unsigned long timeout_ms = 2000);

#endif // #ifndef KEGBOT_HTTP_UTIL_H
