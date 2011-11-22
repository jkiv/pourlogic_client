// See LICENSE.txt for license details.

#include "IPUtil.h"

/*! \brief Copies `src' into `dst'.
 */
void ip4_copy(ip4 dest, const ip4 src)
{
  for (int i = 0; i < 4; i++)
    dest[i] = src[i];
}

String ip4_to_str(const ip4 address)
{
  String s = "";
  
  // #.#.#.#
  s += String((unsigned int)address[0]);
  s += '.';
  s += String((unsigned int)address[1]);
  s += '.';
  s += String((unsigned int)address[2]);
  s += '.';
  s += String((unsigned int)address[3]);
  
  return s;
}

boolean str_to_ip4(ip4 &address, String const &s)
{
  boolean success = true;
  String quad;
  int dot_i = 0;
  int quad_start = 0;
  
  // Quick check on string
  success &= s.length() >= 7 && s.length() <= 15; // x.x.x.x, xxx.xxx.xxx.xxx
  
  for (int i = 0; i < 4 && success; i++)
  {
    // Get start of the next quad
    if (i < 3)
    {
      // Get the index of the next dot
      dot_i = s.indexOf('.', quad_start);
      success &= (dot_i != -1);
    }
    else
    {
      // Last quad doesn't end in a dot, use the end of the string
      dot_i = s.length()-1;
    }
    
    if (success)
    {
      // Get the quad
      quad = s.substring(quad_start, dot_i+1);

      // Check quad length
      success &= (quad.length() >= 1 && quad.length() <= 3);

      // Parse quad to number and set the address byte
      uint32_t res = 0;
      success = stringToUnsigned(quad, res);
      success &= res <= 255;
      
      if (success)
        address[i] = (byte) res;
    }
    
    // Remember where the quad starts
    quad_start = dot_i+1;
  }
  
  return success;
}
