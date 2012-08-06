// See LICENSE.txt for license details.

// NOTE -- a lot of this code is messy in order to save RAM and/or time.

#include "PourLogicClient.h"

// Implementation-specific includes
//#include "StringConversion.h"
#include "StreamUtil.h"
#include "HexString.h"

// Strings in program memory to conserve RAM
static const byte PGM_BUFFER_SIZE = 256; //!< Must be larger (+1) than any of the PROGMEM strings

static const char HTTP_STATUS_GET[]          PROGMEM = "GET ";
static const char HTTP_STATUS_POST[]         PROGMEM = "POST ";
static const char HTTP_REQUEST_LINE_TAIL[]   PROGMEM = " HTTP/1.1\r\n";
static const char HEADER_HOST[]              PROGMEM = "Host: " SETTINGS_SERVER_HOSTNAME "\r\n";
static const char HEADER_USER_AGENT[]        PROGMEM = "User-Agent: pourlogic-client/1.0\r\n";
static const char HEADER_CONTENT_TYPE_POST[] PROGMEM = "Content-Type: application/x-www-form-urlencoded\r\n";
static const char HEADER_X_OTP_AUTH[]        PROGMEM = "X-Otp-Auth: ";
static const char HEADER_CONTENT_LENGTH[]    PROGMEM = "Content-Length: ";

static const char POUR_REQUEST_URI[]         PROGMEM = SETTINGS_SERVER_POUR_REQUEST_URI "?";
static const char POUR_REQUEST_PARAM_1[]     PROGMEM = "u=";
static const char POUR_RESULT_URI[]          PROGMEM = SETTINGS_SERVER_POUR_RESULT_URI "?";
static const char POUR_RESULT_PARAM_1[]      PROGMEM = "u=";
static const char POUR_RESULT_PARAM_2[]      PROGMEM = "&v=";
static const char HTTP_STATUS_OK[]           PROGMEM = "200";

static const int MAX_LINE_SIZE   = 256;    //!< Limit to an HTTP line
static const char HTTP_ENDLINE[] = "\r\n";

boolean PourLogicClient::readHTTPLine(String &line) {
  return readStreamUntil(this, HTTP_ENDLINE, line);
}

boolean PourLogicClient::readHTTPLine(String &line, int maximumBytes) {
  return readStreamUntil(this, HTTP_ENDLINE, line, maximumBytes);
}

PourLogicClient::PourLogicClient(String const &key)
{
    // Initialize timeout
    setTimeout(5000);
    
    // Initialize key
    Sha256.init();
    Sha256.print(key);
    memcpy(_key, Sha256.result(), keySize());
}

PourLogicClient::~PourLogicClient() {
  stop();
}

void PourLogicClient::printXOtpAuthHeader(char* pgmbuffer, int id, unsigned long counter, String const& hmac) {
  strncpy_P(pgmbuffer, HEADER_X_OTP_AUTH, PGM_BUFFER_SIZE);
  print(pgmbuffer);
  
  print(id);
  print(':');
  print(counter);
  print(':');
  print(hmac);
  
  strncpy_P(pgmbuffer, HTTP_ENDLINE, PGM_BUFFER_SIZE);
  print(pgmbuffer);
}

/*!
 * Signs a pour request. Returns HMAC via String& hmac.
 * Hashes:
 * <pre>
 *   COUNT\n
 *   QUERY_STRING\n
 * </pre>
 */
void PourLogicClient::signPourRequest(char* pgmbuffer, String const& tagData, String& hmac) {
  Sha256.initHmac(key(), keySize());
  
  // COUNTER\n
  Sha256.println(_otp.count());
  
  // QUERY_STRING\n
  strcpy_P(pgmbuffer, POUR_REQUEST_PARAM_1);
  Sha256.print(pgmbuffer);
  Sha256.println(tagData);
  
  hmac = bytesToHexString(Sha256.resultHmac(), keySize());
}

/*!
 * Signs a pour result.
 * Hashes:
 * <pre>
 *   COUNT\n
 *   QUERY_STRING\n
 * </pre>
 */
void PourLogicClient::signPourResult(char* pgmbuffer, String const& tagData, int const& volume_mL, String& hmac) {
  Sha256.initHmac(key(), keySize());
  
  // COUNTER\n
  Sha256.println(_otp.count());
  
  // QUERY_STRING\n
  strcpy_P(pgmbuffer, POUR_REQUEST_PARAM_1);
  Sha256.print(pgmbuffer);
  Sha256.println(tagData);
  
  strcpy_P(pgmbuffer, POUR_RESULT_PARAM_2);
  Sha256.print(pgmbuffer);
  Sha256.println(volume_mL);
  
  hmac = bytesToHexString(Sha256.resultHmac(), keySize());
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
boolean PourLogicClient::sendPourRequest(String const &tagData) {
    // Initialize PROGMEM string buffer
    char buffer[PGM_BUFFER_SIZE];
    buffer[PGM_BUFFER_SIZE-1] = '\0';
    String hmac;
    
    // Initialize OTP for this request
	//   (new request warrants a new token)
    _otp.increment();
    
    // Sign message
    signPourRequest(buffer, tagData, hmac);
    
    // HTTP status line
    strcpy_P(buffer, HTTP_STATUS_GET);
    print(buffer);
    strcpy_P(buffer, POUR_REQUEST_URI);
    print(buffer);
    
    strcpy_P(buffer, POUR_REQUEST_PARAM_1);
    print(buffer);
    print(tagData);
    
    strcpy_P(buffer, HTTP_REQUEST_LINE_TAIL);
    print(buffer);
    print(HTTP_ENDLINE);
    // -- Status line done
    
    // We would construct and hash the message body before the headers so
    // we can send X-Otp-Auth, but alas it is an empty body, so we omit it.
    
    // HTTP headers
    // .. Host
    strcpy_P(buffer, HEADER_HOST);
    print(buffer);
    
    // .. User-Agent
    strcpy_P(buffer, HEADER_USER_AGENT);
    print(buffer);
    
    // ... X-OTP-AUTH
    printXOtpAuthHeader(buffer, id(), (unsigned long) _otp.count(), hmac);
    
    // Empty body
    strcpy_P(buffer, HTTP_ENDLINE);
    print(buffer);
    
    return true;
}

/*! A pour request response includes a body in the following format:
 *  <pre>
 *    MAX VOLUME\n
 *  </pre>
 *
 * MAX VOLUME is a integer in ASCII representation.
 *
 * The HTTP response must also include the X-Otp-Auth header to verify
 * that the response is from the previous request.  The HMAC provided
 * is the keyed hash of the following:
 *
 * <pre>
 *   COUNTER\n
 *   STATUS\n
 *   RESPONSE BODY
 * </pre>
 *
 * COUNTER is the ASCII representation of the OTP counter.
 */
boolean PourLogicClient::getPourRequestResponse(int& maxVolume)
{ 
    String messageHmac;
    maxVolume = 0;
    
    // Handle status line
    if (!checkResponseStatusLine(HTTP_STATUS_OK)) {
      return false;
    }
	
    // Read headers
    parseResponseHeaders(messageHmac);

    // Read message body
    // .. maxVolume
    maxVolume = parseInt();
    
    // Close connection
    stop();
	
    // Verify HMAC
    Sha256.initHmac(key(), keySize());
    Sha256.print((unsigned long) _otp.count());      // server should have the same count (not provided in response)
    Sha256.print('\n');
    Sha256.print(HTTP_STATUS_OK);
    Sha256.print('\n');
    Sha256.print(maxVolume);
    Sha256.print('\n');
	
    return bytesToHexString(Sha256.resultHmac(), keySize()).equalsIgnoreCase(messageHmac);
}
/*
boolean PourLogicClient::getPourRequestResponse(int &maxVolume)
{
    const int MAX_MESSAGE_SIZE = 256; // Limit to the response size
    const char statusOK[] = "200";
    
    String message;
    String messageHmac;
    
    // Begin HMAC now so we can HMAC while we parse
    Sha256.initHmac(key(), keySize());
    Sha256.print((unsigned long) _otp.count());
    Sha256.print('\n');
    
    ////// Read status line
    
    if (!readHTTPLine(message, MAX_MESSAGE_SIZE)) {
      return false; // Could not read status line
    }
    
    ////// Verify status line
    
    if (message.indexOf(statusOK) < 0) {
        return false;
    }
    
    Sha256.print(statusOK);
    
    ////// Read headers
    
    messageHmac = "";
    
    for(;;) {
      
      // Read the next HTTP line
      message = "";
      if (!readHTTPLine(message, MAX_MESSAGE_SIZE)) {
        return false; // could not read line
      }

      // Check headers
      // .. Quit on empty line (i.e. \r\n)
      if (message.startsWith(HTTP_ENDLINE)) {
        break;
      }
      
      // .. grab the response HMAC
      if (messageHmac.startsWith(HEADER_OTP_AUTH)) {
        messageHmac = getOTPHeaderHmac(message);
      }
    }

    ////// Read message body
    
    message = "";
     
    // .. maxVolume
    if (!readStreamUntil(this, "\n", message, MAX_MESSAGE_SIZE)) {
      return false;
    }
    
    message.trim();
    
    // .. maxVolume - validate
    if (message.length() == 0) {
      return false;
    }
    
    // .. maxVolume - convert to correct type
    if (!stringToUnsigned(message, maxVolume)) {
      return false;
    }
    
    // .. maxVolume - add to HMAC
    Sha256.print(message);
    
    ////// Verify HMAC
    
    if (!bytesToHexString(Sha256.resultHmac(), keySize()).equalsIgnoreCase(messageHmac)) {
      return false;
    }
    
    // Close connection
    stop();
    
    return true;
}
*/


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
boolean PourLogicClient::sendPourResult(String const &tagData, int pourVolume)
{
    // Initialize PROGMEM string buffer
    char buffer[PGM_BUFFER_SIZE];
    buffer[PGM_BUFFER_SIZE-1] = '\0';
    String hmac;
    int contentLength = 0;
    
    // Initialize OTP for this request
    _otp.increment(); // new request warrants a new token
    
    // We create HMAC in its own step, duplicating a lot of
    // strcpy_P... but this is also so we can calculate the
    // content length for the 'Content Length' header.

    // Create HMAC
    signPourResult(buffer, tagData, pourVolume, hmac);
    
    // Request line
    strcpy_P(buffer, HTTP_STATUS_POST);
    print(buffer);
    
    strcpy_P(buffer, POUR_RESULT_URI);
    print(buffer);
    
    strcpy_P(buffer, HTTP_REQUEST_LINE_TAIL);
    print(buffer);

    // Headers
    // .. Host
    strcpy_P(buffer, HEADER_HOST);
    print(buffer);
    
    // .. User-Agent
    strcpy_P(buffer, HEADER_USER_AGENT);
    print(buffer);
    
    // .. X-Otp-Auth
    printXOtpAuthHeader(buffer, id(), (unsigned long) _otp.count(), hmac);
    
    // .. Content-Type
    strcpy_P(buffer, HEADER_CONTENT_TYPE_POST);
    print(buffer);
    
    // .. Content-Length
    contentLength += 2 + tagData.length(); // u= ...
    contentLength += 3 + String(pourVolume).length(); // &v= ...
    
    strcpy_P(buffer, HEADER_CONTENT_LENGTH);
    print(buffer);
    print(contentLength);
    strcpy_P(buffer, HTTP_ENDLINE);
    print(buffer);
    print(buffer);

    // Body
    strcpy_P(buffer, POUR_RESULT_PARAM_1);
    print(buffer);
    print(tagData);
    
    strcpy_P(buffer, POUR_RESULT_PARAM_2);
    print(buffer);
    print(pourVolume);

    return true;
} 

/*
boolean PourLogicClient::getPourResultResponse()
{ 
  // TODO -- this function is a lot like the other response, only it doesn't handle the message body... HINT?
    const int MAX_MESSAGE_SIZE = 256; // Limit to the response size
    const char statusOK[] = "200";
    
    String message;
    String messageHmac;
    
    // Begin HMAC now so we can HMAC while we parse
    Sha256.initHmac(key(), keySize());
    Sha256.print((unsigned long) _otp.count());
    Sha256.print('\n');
    
    ////// Read status line

    if (!readHTTPLine(message, MAX_MESSAGE_SIZE)) {
      return false; // Could not read status line
    }
    
    ////// Verify status line
    
    if (message.indexOf(statusOK) < 0) {
        return false;
    }
    
    Sha256.print(statusOK);
    
    ////// Read headers
    
    messageHmac = "";
    
    for(;;) {
      
      // Read the next HTTP line
      message = "";
      if (!readHTTPLine(message, MAX_MESSAGE_SIZE)) {
        return false;
      }

      // Check headers
      // .. Quit on empty line (i.e. \r\n)
      if (message.startsWith(HTTP_ENDLINE)) {
        break;
      }
      
      // .. grab the response HMAC
      if (messageHmac.startsWith(HEADER_OTP_AUTH)) {
        messageHmac = getOTPHeaderHmac(message);
      }
    }

    ////// Read message body
    // (No message body)
    
    ////// Verify HMAC
    
    if (!bytesToHexString(Sha256.resultHmac(), keySize()).equalsIgnoreCase(messageHmac)) {
      return false;
    }
    
    // Close connection
    stop();
    
    return true;
}
*/

boolean PourLogicClient::getPourResultResponse()
{ 
    String messageHmac;
    
    // Handle status line
    if (!checkResponseStatusLine(HTTP_STATUS_OK)) {
      return false;
    }
	
    // Read headers
    parseResponseHeaders(messageHmac);

    // Read message body
    //   (No message body)
    
    // Close connection
    stop();
	
    // Verify HMAC
    Sha256.initHmac(key(), keySize());
    Sha256.print((unsigned long) _otp.count());      // server should have the same count (not provided in response)
    Sha256.print('\n');
    Sha256.print(HTTP_STATUS_OK);
    Sha256.print('\n');
    // (no message body)
    Sha256.print('\n');
	
    return bytesToHexString(Sha256.resultHmac(), keySize()).equalsIgnoreCase(messageHmac);
}

/*!
 * Grab the HMAC from an X-Otp-Auth header line
 * TODO -- counter
 */
boolean PourLogicClient::parseXOtpAuthHeader(String const &line, String &hmac_result) {
  char buffer[13]; // "X-Otp-Auth: " + null
  strcpy_P(buffer, HEADER_X_OTP_AUTH);
  buffer[12] = '\0';
  
  int otpHeaderStart = line.indexOf(buffer);
  int otpHeaderHmacEnd = line.indexOf(HTTP_ENDLINE, otpHeaderStart);
  int otpHeaderHmacStart = line.indexOf(':', otpHeaderStart);
  
  if (otpHeaderStart < 0 || otpHeaderHmacEnd < 0 || otpHeaderHmacStart < 0) {
    return false; // No header or HMAC
  }
  
  // Strip out all but the HMAC
  hmac_result = line.substring(otpHeaderHmacStart+1, otpHeaderHmacEnd);
  hmac_result.trim();
  
  return (hmac_result.length() == 2*keySize());
}

boolean PourLogicClient::checkResponseStatusLine(const char* expected_status) {
  String line;
  
  // Read status line
  if (!readHTTPLine(line, MAX_LINE_SIZE)) {
    return false; // Could not read status line
  }
  
  return line.indexOf(expected_status) >= 0;
}

boolean PourLogicClient::parseResponseHeaders(String &hmac) {
  String line;
  
  for(;;) {
    // Read the next HTTP line
    line = "";
    if (!readHTTPLine(line, MAX_LINE_SIZE)) {
      return false;
    }

    // Check headers
    // .. quit on empty line (i.e. \r\n)
    if (line.startsWith(HTTP_ENDLINE)) {
      break;
    }
      
    // .. grab the response id, HMAC, and count
    if (line.startsWith(HEADER_X_OTP_AUTH)) {
      parseXOtpAuthHeader(line, hmac);
    }
  }
  
  return true;
}

//!< Request the max. volume for a pour for the user given by tagData.
boolean PourLogicClient::requestMaxVolume(String const& tagData, int& maxVolume_mL) {
  maxVolume_mL = 0;
  
  // Request pour
  if (!connect(SETTINGS_SERVER_HOSTNAME, SETTINGS_SERVER_PORT) ||
      !sendPourRequest(tagData)                                ||
      !getPourRequestResponse(maxVolume_mL))
  {
    stop();
    return false;
  }
  
  stop();
  return true;
}

//!< Send the result of a pour to the server.
boolean PourLogicClient::reportPouredVolume(String const& tagData, int const& volume_mL) {
  // Send pour results
  if (!connect(SETTINGS_SERVER_HOSTNAME, SETTINGS_SERVER_PORT) ||
      !sendPourResult(tagData, volume_mL)                      ||
      !getPourResultResponse())
  {
    stop();
    return false;
  }
  
  stop();
  return true;
}

