// See LICENSE.txt for license details.

#include "StreamUtil.h"

//bool readStreamUntil(Stream* stream, String const &pattern, String &body, int maximumBytes) {
bool readStreamUntil(Stream* stream, String const &pattern, unsigned short maximumBytes, char *body) {
  if (!stream) {
    return false;
  }
  
  bool matched = false;
  int matchCount = 0; // consecutive matches
  int matchStart = 0;
  char c = '\0';
  unsigned short bytesRead = 0;
 
  if (pattern.length() <= 0) return true;
  
  // Nullify body (prevent writing) if maximumBytes is zero
  body = (maximumBytes <= 0) ? NULL : body;
  // Null-terminate the body
  if (NULL != body) {
    body[0] = '\0';
  }
  
  while (waitForAvailable(stream) && !matched && (maximumBytes == 0 || bytesRead < maximumBytes)) {
    c = stream->read(); // read byte from stream
    
    if (NULL != body) {
      body[bytesRead++] = c; // append character to string
    }
    
    if (c == pattern.charAt(matchCount)) {
      matchCount++; // increment consecutive matches
    }
    else {
      matchCount = 0; // reset count of consecutive matches
    }
    
    matched = (matchCount == pattern.length()); // matched is true when we have read the pattern, false otherwise
  }
  
  // Null-terminate
  if (NULL != body) {
    body[bytesRead] = '\0';
  }
  
  return matched;
}

/*
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
    Serial.print(c); // FIXME remove

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
*/

bool waitForAvailable(Stream* stream, unsigned long timeout_ms) {
  unsigned long startTime = millis();
  
  while (!stream->available()) {
    if ((millis() - startTime) > timeout_ms) {
      return false; // Timeout
    }
  }
  
  return true;
}

/*
inline bool readStreamWhileIn(Stream* stream, String const &alphabet) {
  return _readStreamWhile(stream, alphabet, true);
}*/

bool readStreamWhileIn(Stream* stream, String const &alphabet, int maximumBytes, char* body) {
  return _readStreamWhile(stream, alphabet, true, maximumBytes, body);
}

/*
inline bool readStreamWhileNotIn(Stream* stream, String const &alphabet) {
  return _readStreamWhile(stream, alphabet, false);
}*/

bool readStreamWhileNotIn(Stream* stream, String const &alphabet, unsigned short maximumBytes, char* body) {
  return _readStreamWhile(stream, alphabet, false, maximumBytes, body);
}

/*
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
    Serial.print(c); // FIXME remove

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
}*/

bool _readStreamWhile(Stream* stream, String const &alphabet, bool whileInAlphabet, int maximumBytes, char* body) {
  if (!stream) {
    return false;
  }
  
  if (alphabet.length() <= 0) {
    return false;
  }
  
  char c = '\0';
  bool isInAlphabet = false;
  unsigned short bytesRead = 0;
  
  body = (maximumBytes == 0) ? NULL : body;
  
  while (waitForAvailable(stream)) {
    // Peek next byte (we leave it if it breaks this loop)
    c = stream->peek();
    
    // Search for character in alphabet
    isInAlphabet = (alphabet.indexOf(c) >= 0);

    //if ((whileInAlphabet && !isInAlphabet) || (!whileInAlphabet && isInAlphabet)) {
    if (whileInAlphabet ^ isInAlphabet) {
      break;
    }

    // Continue reading...
    stream->read();
    if (NULL != body) {
      body[bytesRead] = c;
    }
    bytesRead++;
  }
  
  // Null-terminate body
  if (NULL != body) {
    body[bytesRead] = '\0'; 
  }
  
  return (stream->available() > 0); // If there is stuff left to read, we assume we "broke" above
}

void readUntilUnavailable(Stream* stream){
  while(stream->available()) {
    stream->read();
  }
}
