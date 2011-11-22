// See LICENSE.txt for license details.

#include "StringConversion.h"
#include "HexString.h"

boolean stringToUnsigned(String const &s, uint32_t &res)
{
  uint32_t overflower = 0;
  char c = '\0';
  
  for (int i = 0; i < s.length(); i++)
  {
    c = s.charAt(i);
    
    // Validate string at the same time
    if (c < '0' || c > '9')
    {
      Serial.println(c);
      res = 0;
      return false; // not a number
    }

    // FIXME -- overflow detection does not work...
    /*
    // Append numerical values
    overflower = res; // detect overflow
    res *= 10;
    
    if (overflower > res)
    {
      res = 0;
      return false; // overflow
    }
    
    res += c - '0';
    
    if (overflower > res)
    {
      res = 0;
      return false; // overflow
    }
    */
  }
  
  return true;
}

boolean stringToSigned(String const &s, int32_t &res)
{
  int32_t overflower = 0;
  char c = '\0';
  
  for (int i = 0; i < s.length(); i++)
  {
    c = s.charAt(i);
    
    // Validate string at the same time
    if (c < '0' || c > '9')
    {
      res = 0;
      return false; // not a number
    }

    // Append numerical values
    overflower = res; // detect overflow
    res *= 10;
    res += c - '0';
    
    if ((overflower < 0 && overflower < res) ||
        (overflower > 0 && overflower > res))
    {
      res = 0;
      return false; // overflow
    }
  }
  
  return true;
}
