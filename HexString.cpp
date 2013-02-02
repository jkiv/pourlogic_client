// See LICENSE.txt for license details.

#include "HexString.h"

String bytesToHexString(const byte *bytes, int length) {
    String hex_string = "";
    if (length <= 0) return hex_string;

    for (int i = 0; i < length; i++) {
      hex_string += "0123456789abcdef"[bytes[i]>>4];
      hex_string += "0123456789abcdef"[bytes[i]&0xf];
    }
    
    return hex_string;
}

void bytesToHexString(char* result_string, const byte *bytes, int length) {    
    result_string[0] = '\0';
    
    if (length <= 0) return;

    for (int i = 0; i < length; i++) {
      result_string[(i<<1)]   = "0123456789abcdef"[bytes[i]>>4];
      result_string[(i<<1)+1] = "0123456789abcdef"[bytes[i]&0xf];
    }
    
    result_string[length<<1] = '\0';
}

boolean hexStringToBytes(String const &hex_string, byte *buffer, int &result_length, int max_length) {
  
    char c = '\0';
    byte tmp = 0;
    int i = 0, j = 0;
    
    result_length = 0;

    if (hex_string.length() <= 0) {
        return true; // result_length == 0
    }
    
    if (hex_string.length() % 2 != 0) {
        return false; // Contains a half-byte, cannot parse
    }
    
    // Accept strings that start with 0x
    if (hex_string.startsWith("0x")) {
        i = 2;
    }

    // Test hex string before parsing    
    // .. by checking its length
    result_length = hex_string.length()*sizeof(byte)/2;
    
    if (result_length > max_length) {
      result_length = 0;
      return false;
    }
    
    // .. by checking its character set
    for (int j = i; j < hex_string.length(); j++) {
        c = hex_string.charAt(j);
        
        if (!((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || (c >= '0' && c <= '9'))) {
            result_length = 0;
            return false;
        }
    }
    
    // Zero byte buffer before populating
    memset(buffer, 0, result_length);

    // Parse hex string into values
    for (i = 0, j = 0; i < hex_string.length(); i++) {
        c = hex_string.charAt(i);

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
