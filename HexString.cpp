// See LICENSE.txt for license details.

#include "HexString.h"

String bytesToHexString(const byte *bytes, int length) {    
    String hexString = "";
    if (length <= 0) return hexString;

    for (int i = 0; i < length; i++) {
      hexString += "0123456789abcdef"[bytes[i]>>4];
      hexString += "0123456789abcdef"[bytes[i]&0xf];
    }
    
    return hexString;
}

void bytesToHexString(char* result_string, const byte *bytes, int length) {    
    //String hexString = "";
    result_string[0] = '\0';
    
    if (length <= 0) return;

    for (int i = 0; i < length; i++) {
      result_string[(i<<1)]   = "0123456789abcdef"[bytes[i]>>4];
      result_string[(i<<1)+1] = "0123456789abcdef"[bytes[i]&0xf];
    }
    
    result_string[length<<1] = '\0';
}

boolean hexStringToBytes(String const &hexString, byte *buffer, int &resultLength, int maxLength) {
  
    char c = '\0';
    byte tmp = 0;
    int i = 0, j = 0;
    
    resultLength = 0;

    if (hexString.length() <= 0) {
        return true; // resultLength == 0
    }
    
    if (hexString.length() % 2 != 0) {
        return false; // Contains a half-byte, cannot parse
    }
    
    // Accept strings that start with 0x
    if (hexString.startsWith("0x")) {
        i = 2;
    }

    // Test hex string before parsing    
    // .. by checking its length
    resultLength = hexString.length()*sizeof(byte)/2;
    
    if (resultLength > maxLength) {
      resultLength = 0;
      return false;
    }
    
    // .. by checking its character set
    for (int j = i; j < hexString.length(); j++) {
        c = hexString.charAt(j);
        
        if (!((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || (c >= '0' && c <= '9'))) {
            resultLength = 0;
            return false;
        }
    }
    
    // Zero byte buffer before populating
    memset(buffer, 0, resultLength);

    // Parse hex string into values
    for (i = 0, j = 0; i < hexString.length(); i++) {
        c = hexString.charAt(i);

        // Uppercase
        if (c >= 'A' && c <= 'F') {
            tmp = c - 'A' + 10;
        }
        // Lowercase
        else if (c >= 'a' && c <= 'f') {
            tmp = c - 'a' + 10; 
        }
        // Numbers
        else if (c >= '0' && c <= '9') {
            tmp = c - '0';
        }

        if ((i % 2) == 0) {
            // High byte
            buffer[j] = tmp<<4;
        }
        else {
            // Low byte
            buffer[j] |= tmp;
            j++;
        }
    }

    return true;
}
