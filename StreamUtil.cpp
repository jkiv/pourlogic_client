// See LICENSE.txt for license details.

#include "StreamUtil.h"

bool readStreamUntil(Stream* stream, String const &pattern, String &body, int maximumBytes) {
  if (!stream) {
    return false;
  }
  
  bool matched = false;
  int matchCount = 0; // consecutive matches
  int matchStart = 0;
  char c = '\0';
  int bytesRead = 0;
 
  if (pattern.length() <= 0) return true;
  
  while (waitForAvailable(stream) && !matched && (maximumBytes <= 0 || bytesRead < maximumBytes)) {
    c = stream->read(); // read byte from stream

    body.concat(c); // append character to string
    bytesRead++;
    
    if (c == pattern.charAt(matchCount)) {
      matchCount++; // increment consecutive matches
    }
    else {
      matchCount = 0; // reset count of consecutive matches
    }
    
    matched = (matchCount == pattern.length()); // matched is true when we have read the pattern, false otherwise
  }
  
  return matched;
}

bool readStreamUntil(Stream* stream, String const &pattern) {
  if (!stream) return false;
  
  bool matched = false;
  int matchCount = 0;
  int matchStart = 0;
  char c = '\0';
 
  if (pattern.length() <= 0) {
    return true;
  }
  
  while (waitForAvailable(stream) && !matched) {
    c = stream->read();
    
    if (c == pattern.charAt(matchCount)) {
      matchCount++;
    }
    else {
      matchCount = 0;
    }
    
    matched = (matchCount == pattern.length());
  }
  
  return matched;
}


bool waitForAvailable(Stream* stream, unsigned long timeout_ms) {
  unsigned long startTime = millis();
  
  while (!stream->available()) {
    if ((millis() - startTime) > timeout_ms) {
      return false; // Timeout
    }
  }
  
  return true;
}

bool readStreamWhileIn(Stream* stream, String const &alphabet) {
  return _readStreamWhile(stream, alphabet, true);
}

bool readStreamWhileIn(Stream* stream, String const &alphabet, String &body, int maximumBytes) {
  return _readStreamWhile(stream, alphabet, true, body, maximumBytes);
}

bool readStreamWhileNotIn(Stream* stream, String const &alphabet) {
  return _readStreamWhile(stream, alphabet, false);
}

bool readStreamWhileNotIn(Stream* stream, String const &alphabet, String &body, int maximumBytes) {
  return _readStreamWhile(stream, alphabet, false, body, maximumBytes);
}

bool _readStreamWhile(Stream* stream, String const &alphabet, bool whileInAlphabet) {
  if (!stream) {
    return false;
  }
  
  if (alphabet.length() <= 0) {
    return false;
  }
  
  char c = '\0';
  bool isInAlphabet = false;
  
  while (waitForAvailable(stream)) {
    c = stream->peek(); // peek byte from stream

    isInAlphabet = false;
    
    // Search for character in alphabet
    for (int i = 0; i < alphabet.length(); i++) {
      if (c == alphabet.charAt(i)) {
        isInAlphabet = true;
        break;
      }
    }

    if ((whileInAlphabet && !isInAlphabet) || (!whileInAlphabet && isInAlphabet)) {
      return true;
    }

    // Continue reading...
    stream->read();
  }
  
  return false;
}

bool _readStreamWhile(Stream* stream, String const &alphabet, bool whileInAlphabet, String &body, int maximumBytes) {
  if (!stream) {
    return false;
  }
  
  if (alphabet.length() <= 0) {
    return false;
  }
  
  char c = '\0';
  bool isInAlphabet = false;
  
  while (waitForAvailable(stream)) {
    c = stream->peek(); // peek byte from stream

    isInAlphabet = false;
    
    // Search for character in alphabet
    for (int i = 0; i < alphabet.length(); i++) {
      if (c == alphabet.charAt(i)) {
        isInAlphabet = true;
        break;
      }
    }

    if ((whileInAlphabet && !isInAlphabet) || (!whileInAlphabet && isInAlphabet)) {
      return true;
    }

    // Continue reading...
    body.concat(c);
    stream->read();
  }
  
  return false;
}
