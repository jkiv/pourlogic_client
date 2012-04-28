// See LICENSE.txt for license details.

// TODO -- we might be able to reduce the number of calls to print, sha256.print, etc. somehow.
//         doing so may improve out memory footprint.

#include "PourLogicClient.h"

// Implementation-specific includes
#include <sha256.h>

#include "String.h"
#include "StreamUtil.h"
#include "HexString.h"

// Strings in program memory to conserve RAM
static const byte PGM_BUFFER_SIZE = 41;
static const char STATUS_GET[] PROGMEM = "GET";
static const char STATUS_POST[] PROGMEM = "POST";
static const char STATUS_TAIL[] PROGMEM = "HTTP/1.1";
static const char HEADER_HOST[] PROGMEM = "Host: ";
static const char HEADER_CONTENT_TYPE[] PROGMEM = "Content-Type: ";
static const char HEADER_CONTENT_TYPE_POST[] PROGMEM = "application/x-www-form-urlencoded";
static const char HEADER_CONTENT_LENGTH[] PROGMEM = "Content-Length: ";
static const char HEADER_POURLOGIC_USER_AGENT[] PROGMEM = "User-Agent: pourlogic-client/1.0";
static const char HEADER_OTP_AUTH[] PROGMEM = "X-Otp-Auth: ";

static const char HTTP_ENDLINE[] = "\r\n";

bool PourLogicClient::readHTTPLine(String &line)
{
  return readStreamUntil(this, HTTP_ENDLINE, line);
}

bool PourLogicClient::readHTTPLine(String &line, int maximumBytes)
{
  return readStreamUntil(this, HTTP_ENDLINE, line, maximumBytes);
}

PourLogicClient::PourLogicClient(String const &key)
{
    // Initialize persistent counter
    _otp.begin(0);
    
    // Initialize key
    Sha256.init();
    Sha256.print(key);
    memcpy(_key, Sha256.result(), 32);
}

PourLogicClient::~PourLogicClient()
{
  stop();
}

/*! A pour request is an HTTP GET request with the following parameters:
 *   - keg ID (k)
 *   - user's RFID tag data (u)
 *
 * The HTTP request must also include the X-Otp-Auth header
 * whose HMAC is the result of hashing the OTP counter, the request method
 * and the query string (everything after the ? in the URI), and the message
 * body separated by newlines:
 *
 * <pre>
 *   COUNTER\n
 *   METHOD\n
 *   [QUERY STRING]\n
 *   [MESSAGE BODY]
 * </pre>
 *
 * QUERY STRING and MESSAGE BODY are optional if they don't exit.  However,
 * the newline at the end of QUERY STRING should be included even if there
 * is no query string.
 */
bool PourLogicClient::sendPourRequest(String const &tagData)
{
    char buffer[PGM_BUFFER_SIZE+1];
    buffer[PGM_BUFFER_SIZE] = '\0';
 
    incrementOTPCounter(); // new request warrants a new token
    
    // Request line
    strcpy_P(buffer, STATUS_GET);
    print(buffer);
    print(' ');
    print(requestUri());
    print('?');
    
    // .. start hashing as we go
    Sha256.initHmac(key(), keySize());
    Sha256.print(getOTPCounter()); // HMAC - COUNTER
    Sha256.print('\n');
    Sha256.print(buffer); // HMAC - method
    Sha256.print('\n');
    
    // HMAC - query string
    print('k');
    print('=');
    print(id());
    
    Sha256.print('k');
    Sha256.print('=');
    Sha256.print(id());
    
    print('&');
    print('u');
    print('=');
    print(tagData);
    
    Sha256.print('&');
    Sha256.print('u');
    Sha256.print('=');
    Sha256.print(tagData);
    
    Sha256.print('\n');
    
    // HMAC - message body (NONE)
    
    print(' ');
    strcpy_P(buffer, STATUS_TAIL);
    print(buffer);
    print(HTTP_ENDLINE);
    
    // Headers
    // .. Host
    strcpy_P(buffer, HEADER_HOST);
    print(buffer);
    print(serverHostname());
    print(HTTP_ENDLINE);
    
    // .. User-Agent
    strcpy_P(buffer, HEADER_POURLOGIC_USER_AGENT);
    print(buffer);
    print(HTTP_ENDLINE);
    
    // ... X-OTP-AUTH
    strcpy_P(buffer, HEADER_OTP_AUTH);
    print(buffer);
    print(' ');
    print(id());
    print(':');
    print(getOTPCounter());
    print(':');
    print(bytesToHexString(Sha256.resultHmac(), 32));
    print(HTTP_ENDLINE);
    
    // TODO other headers?
    
    // Empty body
    print(HTTP_ENDLINE);

    return true;
}

/*! A pour request response includes a body in the following format:
 *  <pre>
 *    MAX VOLUME
 *  </pre>
 *
 * MAX VOLUME is an unsigned 32-bit number in ASCII representation.
 *
 * The HTTP response must also include the X-OTP-AUTH header to verify
 * that the response is from the previous request.  The HMAC provided
 * is the keyed hash of the following:
 *
 * <pre>
 *   COUNTER\n
 *   MESSAGE BODY
 * </pre>
 *
 * COUNTER is the ASCII representation of the OTP counter.
 */

bool PourLogicClient::getPourRequestResponse(uint32_t &maxVolume)
{
    const int MAX_MESSAGE_SIZE = 256; // Limit to the response size
    const char statusOK[] = "200";

    String message;
    String messageHmac;
    
    bool success = false;
    
    // Begin HMAC so we can HMAC while we parse
    Sha256.initHmac(key(), keySize());
    Sha256.print(getOTPCounter());
    Sha256.print('\n');
    
    // Read status line
    if (!readyOrTimeout(this))
    {
      return false;
    }

    if (!readHTTPLine(message)) // TODO limits
    {
      return false; // Could not read status line
    }
    
    // Check status line
    if (message.indexOf(statusOK) < 0)
    {
        return false;
    }
    
    // Read headers
    messageHmac = "";
    
    while(success = readyOrTimeout(this)) {
      message = "";
      
      if (!readHTTPLine(message)) {
        success = false;
        break;
      }

      // Quit on empty line (i.e. \r\n)
      if (message.length() == 2) {
        success = true;
        break;
      }

      // Read AUTH header
      if (messageHmac.length() == 0) {
        messageHmac = getOTPHeaderHmac(message);
      }
    }

    if (!success) {
      return false; // Could not read headers
    }
    
    message = ""; // Clear the message

    // Read message body
    if (!readyOrTimeout(this))
    {
      return false;
    }
        
    // .. maxVolume
    readStreamUntil(this, "\n", message);
    Sha256.print(message);
    Serial.println(message); // TODO REMOVE
    message.trim();
    
    if (message.length() == 0)
    {
      maxVolume = 0;
    }
    else
    {
      if(!stringToUnsigned(message, maxVolume))
      {
        return false;
      }
        
    }
    
    // Done reading ethernet..
    stop();
    
    /*
    // Reconstruct HMAC
    if (!bytesToHexString(Sha256.resultHmac(), 32).equalsIgnoreCase(messageHmac))
    {
      Serial.println("FAILED HMAC.");
      return false; // Bad HMAC
    }
    */
    
    return true;
}


/*! A pour result is an HTTP POST request with the following parameters:
 *   - keg ID (k)
 *   - user's RFID tag data (u)
 *   - the volume of the pour (v)
 *
 * The HTTP request must also include the X-OTP-AUTH header
 * whose HMAC is the result of hashing the OTP counter, the request method
 * and the query string (everything after the ? in the URI), and the message
 * body separated by newlines:
 *
 * <pre>
 *   COUNTER\n
 *   METHOD\n
 *   [QUERY STRING]\n
 *   [MESSAGE BODY]
 * </pre>
 *
 * QUERY STRING and MESSAGE BODY are optional if they don't exit.  However,
 * the newline at the end of QUERY STRING should be included even if there
 * is no query string.
 *
 */
bool PourLogicClient::sendPourResult(String const &tagData, uint32_t pourVolume)
{
    char buffer[PGM_BUFFER_SIZE+1];
    int contentLength = 0;
    
    buffer[PGM_BUFFER_SIZE] = '\0';
    
    incrementOTPCounter(); // new request warrants a new token
    
    // Create HMAC
    Sha256.initHmac(key(), keySize());
    
    // .. counter
    Sha256.print(getOTPCounter());
    Sha256.print('\n');
    
    // .. method
    strcpy_P(buffer, STATUS_POST);
    Sha256.print(buffer);
    Sha256.print('\n');
    
    // .. query string
    Sha256.print('\n'); // none
    
    // ... message body
    Sha256.print('k');
    Sha256.print('=');
    Sha256.print(id());
    
    contentLength += 2 + String(id()).length();
    
    Sha256.print('&');
    Sha256.print('u');
    Sha256.print('=');
    Sha256.print(tagData);
    
    contentLength += 3 + tagData.length();
    
    Sha256.print('&');
    Sha256.print('v');
    Sha256.print('=');
    Sha256.print(pourVolume);
    
    contentLength += 3 + String(pourVolume).length();

    // Request line
    strcpy_P(buffer, STATUS_POST); // should already be in buffer, no?
    print(buffer);
    print(' ');
    print(resultUri());
    print(' ');
    
    strcpy_P(buffer, STATUS_TAIL);
    print(buffer);
    print(HTTP_ENDLINE);

    // Headers
    // .. Host
    strcpy_P(buffer, HEADER_HOST);
    print(buffer);
    print(serverHostname());
    print(HTTP_ENDLINE);
    
    // .. User-Agent
    strcpy_P(buffer, HEADER_POURLOGIC_USER_AGENT);
    print(buffer);
    print(HTTP_ENDLINE);
    
    // .. X-Otp-Auth
    strcpy_P(buffer, HEADER_OTP_AUTH);
    print(buffer);
    print(id());
    print(':');
    print(getOTPCounter());
    print(':');
    print(bytesToHexString(Sha256.resultHmac(), 32));
    print(HTTP_ENDLINE);
    
    // .. Content-Type
    strcpy_P(buffer, HEADER_CONTENT_TYPE);
    print(buffer);
    strcpy_P(buffer, HEADER_CONTENT_TYPE_POST);
    print(buffer);
    print(HTTP_ENDLINE);
    
    // .. Content-Length
    strcpy_P(buffer, HEADER_CONTENT_LENGTH);
    print(buffer);
    print(contentLength);
    print(HTTP_ENDLINE);
    print(HTTP_ENDLINE);

    // Body
    print('k');
    print('=');
    print(id());
    
    print('&');
    print('u');
    print('=');
    print(tagData);
    
    print('&');
    print('v');
    print('=');
    print(pourVolume);

    return true; // TODO report failure
} 

bool PourLogicClient::getPourResultResponse()
{ 
    const char statusOK[] = "200";
    bool success = true;

    String message = "";
    String messageHmac = "";

    // Wait for data or timeout
    if (!readyOrTimeout(this))
      return false;

    // Read status line
    if (!readHTTPLine(message))  // TODO check limits
        return false; // Could not receive status

    // Check status line
    if (message.indexOf(statusOK) < 0)
    {
        return false; // Received negative response
    }
    
    messageHmac = "";
    
    // Read headers
    while(success = readyOrTimeout(this))
    {   
      message = "";
      
      if (!readHTTPLine(message))
      {
        success = false;
        break;
      }
      
      // Quit on empty line (i.e. \r\n)
      if (message.length() == 2)
      {
        success = true;
        break;
      }

      // Read AUTH header
      if (messageHmac.length() == 0)
      {
        messageHmac = getOTPHeaderHmac(message);
      }
    }

    if (!success)
    {
      return false;
    }
    
    if (messageHmac.length() == 0)
    {
      return false;
    }
    
    // Done reading ..
    stop();
    
    /* TODO fix failure of this
    Sha256.initHmac(_settings.key(), _settings.keySize());
    Sha256.print(getOTPCounter());
    
    if (!bytesToHexString(Sha256.resultHmac(), 32).equalsIgnoreCase(messageHmac))
      return false; // Bad HMAC
    */
    
    return true;
}

String PourLogicClient::getOTPCounter()
{
  return String((unsigned long) _otp.count());
}

void PourLogicClient::incrementOTPCounter()
{
  _otp.increment();
}

/*!
 * Grab the HMAC from an X-Otp-Auth header line
 */
String PourLogicClient::getOTPHeaderHmac(String const &message)
{
  char buffer[13]; // "X-Otp-Auth: " + null
  strcpy_P(buffer, HEADER_OTP_AUTH);
  buffer[12] = '\0';
  
  int otpHeaderStart = message.indexOf(buffer);
  int otpHeaderHmacEnd = message.indexOf(HTTP_ENDLINE, otpHeaderStart);
  int otpHeaderHmacStart = message.indexOf(':', otpHeaderStart);
  
  if (otpHeaderStart < 0 || otpHeaderHmacEnd < 0 || otpHeaderHmacStart < 0)
  {
    return ""; // No header or HMAC
  }
  
  // Strip out all but the HMAC
  String hmac = message.substring(otpHeaderHmacStart+1, otpHeaderHmacEnd);
  hmac.trim();
  
  return hmac;
}
