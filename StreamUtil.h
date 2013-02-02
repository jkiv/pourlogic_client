// See LICENSE.txt for license details.

#ifndef POURLOGIC_STREAM_UTIL_H
#define POURLOGIC_STREAM_UTIL_H

#include <Arduino.h>
#include <Stream.h>
#include <String.h>

/*! \file HTTPUtil.h
 * \brief Utility functions for working with HTTP.
 */

/*!
 * \brief Reads data from #stream into #body until #pattern is witnessed.  #body will contain #pattern.
 * \param stream The Stream to read from; it should be available().
 * \param pattern If this pattern is encountered, reading will stop and the function will return true.
 * \param maximum_bytes The maximum number of bytes to read into before failure. Zero specifies no limit, but nothing will be read into body.
 * \param body All data read will be stored in this character buffer.
 * \return False will be returned if the operation stopped for any reason other than hitting the pattern.  If the pattern was seen, this function returns true.
 */
bool readStreamUntil(Stream* stream, String const &pattern, unsigned short maximum_bytes = 0, char* body = NULL);

/*!
 * \brief Reads data from #stream until #pattern is witnessed.  All read data is discarded.
 * \param stream The Stream to read from; it should be available().
 * \param pattern If this pattern is encountered, reading will stop and the function will return true.
 * \returns False will be returned if the operation stopped for any reason other than hitting the pattern.  If the pattern was seen, this function returns true.
 */
//bool readStreamUntil(Stream* stream, String const &pattern);

/*!
 * \brief Reads data from #stream until the character being read is not in #alphabet.
 * All read data is discarded except the first character that is encountered that is not in the alphabet, which is left as the next readable character.
 * \param stream The Stream to read from.
 * \param alphabet The stream is read until a character that is not in this string is encountered.
 * \returns False will be returned if no character outside of the #alphabet was encountered, true if a character was encountered that was not in the alphabet.
 */
//bool readStreamWhileIn(Stream* stream, String const &alphabet);
inline bool readStreamWhileIn(Stream* stream, String const &alphabet, unsigned short maximum_bytes = 0, char* body = NULL);

/*!
 * \brief Reads data from #stream until the character being read is in #alphabet.
 * All read data is discarded except the first character that is encountered that is not in the alphabet, which is left as the next readable character.
 * \param stream The Stream to read from.
 * \param alphabet The stream is read until a character that is not in this string is encountered.
 * \returns False will be returned if no character outside of the #alphabet was encountered, true if a character was encountered that was not in the alphabet.
 */
//bool readStreamWhileNotIn(Stream* stream, String const &alphabet);
inline bool readStreamWhileNotIn(Stream* stream, String const &alphabet, unsigned short maximum_bytes = 0, char* body = NULL);

//!< Generalized conditional stream reading function used for both #readStreamWhileIn and #readStreamWhileNotIn
//bool _readStreamWhile(Stream* stream, String const &alphabet, bool while_in_alphabet);
bool _readStreamWhile(Stream* stream, String const &alphabet, bool while_in_alphabet, unsigned short maximum_bytes = 0, char* body = NULL);

/*!
 * \brief Waits for data to be ready to read, or times out
 * \param timeout_ms Timeout in milliseconds (default = 2000)
 * \returns True on ready, false on timeout
 */
bool waitForAvailable(Stream* stream, unsigned long timeout_ms = 10000);

/*!
 * \brief Reads the stream until unavailable, discarding all read bytes.
 */
void readUntilUnavailable(Stream* stream);

#endif // #ifndef POURLOGIC_STREAM_UTIL_H
