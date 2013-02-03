// See LICENSE.txt for license details.

#include "StreamUtil.h"

//bool readStreamUntil(Stream& stream, String const &pattern, String &body, int maximum_bytes) {
bool readStreamUntil(Stream& stream, String const &pattern, unsigned short maximum_bytes, char *body) {
  bool matched = false;
  int match_count = 0; // consecutive matches
  int match_start = 0;
  char c = '\0';
  unsigned short bytes_read = 0;
 
  if (pattern.length() <= 0) return true;
  
  // Nullify body (prevent writing) if maximum_bytes is zero
  body = (maximum_bytes <= 0) ? NULL : body;
  // Null-terminate the body
  if (NULL != body) {
    body[0] = '\0';
  }
  
  while (waitForAvailable(stream) && !matched && (maximum_bytes == 0 || bytes_read < maximum_bytes)) {
    c = stream.read(); // read byte from stream
    
    if (NULL != body) {
      body[bytes_read++] = c; // append character to string
    }
    
    if (c == pattern.charAt(match_count)) {
      match_count++; // increment consecutive matches
    }
    else {
      match_count = 0; // reset count of consecutive matches
    }
    
    matched = (match_count == pattern.length()); // matched is true when we have read the pattern, false otherwise
  }
  
  // Null-terminate
  if (NULL != body) {
    body[bytes_read] = '\0';
  }
  
  return matched;
}

bool waitForAvailable(Stream& stream, unsigned long timeout_ms) {
  unsigned long startTime = millis();
  
  while (!stream.available()) {
    if ((millis() - startTime) > timeout_ms) {
      return false; // Timeout
    }
  }
  
  return true;
}

bool readStreamWhileIn(Stream& stream, String const &alphabet, int maximum_bytes, char* body) {
  return _readStreamWhile(stream, alphabet, true, maximum_bytes, body);
}

bool readStreamWhileNotIn(Stream& stream, String const &alphabet, unsigned short maximum_bytes, char* body) {
  return _readStreamWhile(stream, alphabet, false, maximum_bytes, body);
}

bool _readStreamWhile(Stream& stream, String const &alphabet, bool while_in_alphabet, int maximum_bytes, char* body) {
  if (alphabet.length() <= 0) {
    return false;
  }
  
  char c = '\0';
  bool is_in_alphabet = false;
  unsigned short bytes_read = 0;
  
  body = (maximum_bytes == 0) ? NULL : body;
  
  while (waitForAvailable(stream)) {
    // Peek next byte (we leave it if it breaks this loop)
    c = stream.peek();
    
    // Search for character in alphabet
    is_in_alphabet = (alphabet.indexOf(c) >= 0);

    //if ((while_in_alphabet && !is_in_alphabet) || (!while_in_alphabet && is_in_alphabet)) {
    if (while_in_alphabet ^ is_in_alphabet) {
      break;
    }

    // Continue reading...
    stream.read();
    if (NULL != body) {
      body[bytes_read] = c;
    }
    bytes_read++;
  }
  
  // Null-terminate body
  if (NULL != body) {
    body[bytes_read] = '\0'; 
  }
  
  return (stream.available() > 0); // If there is stuff left to read, we assume we "broke" above
}

void readUntilUnavailable(Stream& stream){
  while(stream.available()) {
    stream.read();
  }
}
