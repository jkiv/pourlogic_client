// See LICENSE.txt for license details.

#include "HTTPUtil.h"
#include "StreamUtil.h"

const char HTTP_ENDLINE[]   = "\r\n";
const char HTTP_STATUS_OK[] = "200";

boolean readHTTPLine(Stream& stream, unsigned short maximum_bytes, char* line) {
  return readStreamUntil(stream, HTTP_ENDLINE, maximum_bytes, line);
}

unsigned long printStatusLineHeadGet(Print& target) {
  return target.print(F("GET "));
}

unsigned long printStatusLineHeadPost(Print& target) {
  return target.print(F("POST "));
}

unsigned long printHTTPEndline(Print& target) {
  return target.print(F("\r\n"));
}

unsigned long printStatusLineTail(Print& target) {
  return target.print(F(" HTTP/1.0"));
}

unsigned long printHostHeader(Print& target) {
  unsigned long bytes_sent = 0;

  bytes_sent += target.print(F("Host: "));
  bytes_sent += target.print(F(HTTP_HOSTNAME));
  bytes_sent += printHTTPEndline(target);
  
  return bytes_sent;
}

unsigned long printUserAgentHeader(Print& target) {
  unsigned long bytes_sent = 0;

  bytes_sent += target.print(F("User-Agent: "));
  bytes_sent += target.print(F(HTTP_USER_AGENT));
  bytes_sent += printHTTPEndline(target);
  
  return bytes_sent;
}

unsigned long printContentTypePostHeader(Print& target) {
  unsigned long bytes_sent = 0;

  bytes_sent += target.print(F("Content-Type: application/x-www-form-urlencoded"));
  bytes_sent += printHTTPEndline(target);
  
  return bytes_sent;
}

unsigned long printContentLengthHeader(Print& target, unsigned long content_length) {
  unsigned long bytes_sent = 0;

  bytes_sent += target.print(F("Content-Length: "));
  bytes_sent += target.print(content_length);
  bytes_sent += printHTTPEndline(target);

  return bytes_sent;
}
