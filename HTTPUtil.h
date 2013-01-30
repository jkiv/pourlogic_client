// See LICENSE.txt for license details.

#ifndef HTTP_UTIL_H
#define HTTP_UTIL_H

#include <Arduino.h>

#define HTTP_USER_AGENT "pourlogic/1.0 arduino/1.0"
#define HTTP_HOSTNAME   "pourlogic.com"

//!< Reads an HTTP line or until `maximumBytes' bytes are read.
boolean readHTTPLine(Stream& stream, unsigned short maximumBytes = 0, char* line = NULL);

//!< Prints the start of an HTTP status line for a GET query.
unsigned long printStatusLineHeadGet(Print &target);

//!< Prints the start of an HTTP status line for a POST query.
unsigned long printStatusLineHeadPost(Print &target);

//!< Prints the ending of an HTTP status line.
unsigned long printStatusLineTail(Print &target);

//!< Prints the ending of an HTTP line.
unsigned long printHTTPEndline(Print &target);

//!< Sends a Host header
unsigned long printHostHeader(Print &target);

//!< Sends a User-Agent header for the PourLogic client
unsigned long printUserAgentHeader(Print &target);

//!< Sends a Content-Type header for POST queries
unsigned long printContentTypePostHeader(Print &target);

//!< Sends a Content-Length header
unsigned long printContentLengthHeader(Print &target, unsigned long content_length);

#endif
