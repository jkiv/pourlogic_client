// See LICENSE.txt for license details.

// NOTE -- a lot of this code is messy in order to save RAM and/or time.

#include "PourLogicClient.h"

// Implementation-specific includes
//#include "StringConversion.h"
#include "StreamUtil.h"
#include "HexString.h"

// Strings in program memory to conserve RAM
/*
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
*/

static const int MAX_LINE_SIZE   = 256;    //!< Limit to an HTTP line
static const char HTTP_ENDLINE[] = "\r\n";

boolean PourLogicClient::readHTTPLine(String &line) {
  return readStreamUntil(this, HTTP_ENDLINE, line);
}

boolean PourLogicClient::readHTTPLine(String &line, int maximumBytes) {
  return readStreamUntil(this, HTTP_ENDLINE, line, maximumBytes);
}

//!< Prints the start of an HTTP status line. Separated out since it is duplicated among requests.
unsigned long PourLogicClient::sendStatusLineHeadGet(Print *target) {
  return target->print(F("GET "));
}

//!< Prints the start of an HTTP status line. Separated out since it is duplicated among requests.
unsigned long PourLogicClient::sendStatusLineHeadPost(Print *target) {
  return target->print(F("POST "));
}

//!< Prints the ending of an HTTP status line. Separated out since it is duplicated among requests.
unsigned long PourLogicClient::sendStatusLineTail(Print *target) {
  return target->print(F(" HTTP/1.1"));
}

unsigned long PourLogicClient::sendPourRequestStatusLine(String const& rfid, Print *target) {
  unsigned long bytes_sent = 0;

  bytes_sent += sendStatusLineHeadGet(target);
  bytes_sent += target->print(F(SETTINGS_SERVER_POUR_REQUEST_URI "?" CLIENT_POUR_REQUEST_PARAM_RFID "="));
  bytes_sent += target->print(rfid); // TODO hash rfid?
  bytes_sent += sendStatusLineTail(target);
  
  return bytes_sent;
}

//!< Prints the ending of an HTTP line. Separated out since it is duplicated among requests.
unsigned long PourLogicClient::sendHTTPEndline(Print *target) {
  return target->print(F("\r\n"));
}

unsigned long PourLogicClient::sendPourResultStatusLine(Print *target) {
  unsigned long bytes_sent = 0;
  
  bytes_sent += sendStatusLineHeadGet(target);
  bytes_sent += target->print(F(SETTINGS_SERVER_POUR_RESULT_URI));
  bytes_sent += sendStatusLineTail(target);
  bytes_sent += sendHTTPEndline(target);
  
  return bytes_sent;
}

unsigned long PourLogicClient::sendPourResultMessageBody(String const& rfid, float volume_in_mL, Print *target) {
  unsigned long bytes_sent = 0;
  
  bytes_sent += target->print(F(CLIENT_POUR_RESULT_PARAM_RFID "="));
  bytes_sent += target->print(rfid);
  bytes_sent += target->print(F("&" CLIENT_POUR_RESULT_PARAM_VOLUME "="));
  bytes_sent += target->print(volume_in_mL);
  
  return bytes_sent;
}

void PourLogicClient::sendHostHeader() {
  print(F("Host: "));
  sendHTTPEndline(this);
}

void PourLogicClient::sendUserAgentHeader() {
  print("User-Agent: " CLIENT_USER_AGENT);
  sendHTTPEndline(this);
}

void PourLogicClient::sendContentTypePostHeader() {
  print(F("Content-Type: application/x-www-form-urlencoded"));
  sendHTTPEndline(this);
}

void PourLogicClient::sendContentLengthHeader(unsigned long content_length) {
  // FIXME unsigned long?
  print(F("Content-Length: "));
  print(content_length);
  sendHTTPEndline(this);
}

void PourLogicClient::sendXPourLogicAuthHeader(int bot_id, unsigned long counter, String const& HMAC) {
  print(F("X-PourLogic-Auth: "));
  print(bot_id);
  print(F(":"));
  print(counter);
  print(F(":"));
  print(HMAC);
  sendHTTPEndline(this);
}

void PourLogicClient::initializeAuthHMAC() {
  // Initialize OTP for this request
  _otp.increment();
  Sha256.initHmac(key(), keySize());
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

/*! A pour request is an HTTP GET request with the following parameters:
 *   - user's RFID tag data (u)
 *
 * The HTTP request must also include the X-PourLogic-Auth header in the
 * following format:
 *
 *   X-PourLogic-Auth: ID:COUNTER:HMAC
 *
 * where ID is the client's id, COUNTER is the OTP counter, and HMAC is
 * the result of hashing the request data in the following form:
 *
 * <pre>
 *   COUNTER\n
 *   REQUEST LINE\n
 *   [MESSAGE BODY]
 * </pre>
 *
 * MESSAGE BODY may be empty. If it is empty, then REQUEST LINE should
 * still end in a newline.
 */
boolean PourLogicClient::sendPourRequest(String const &tagData) {
    // Initialize HMAC
    initializeAuthHMAC();
    
    // -- Feed HMAC digest --
    Sha256.println(_otp.count());                  // COUNTER\n
    sendStatusLineHeadGet(&Sha256);                // REQUEST LINE\n
    sendPourRequestStatusLine(tagData, &Sha256);
    sendStatusLineTail(&Sha256);
    Sha256.println();
    // no body
    
    // -- done HMAC
    
    // Send request to server --
    
    // Status/Request line
    sendStatusLineHeadGet(this);
    sendPourRequestStatusLine(tagData, this);   
    sendStatusLineTail(this);
    sendHTTPEndline(this);
    
    // HTTP headers
    sendHostHeader();
    sendUserAgentHeader();
    sendXPourLogicAuthHeader(id(), (unsigned long) _otp.count(), bytesToHexString(Sha256.resultHmac(), keySize()));
    sendContentLengthHeader(0);
    sendHTTPEndline(this);
    sendHTTPEndline(this);
    
    // Message Body
    // .. empty
    
    // -- done sending request 
    
    return true;
}

/*! A pour request response includes a body in the following format:
 *  <pre>
 *    MAX VOLUME\n
 *  </pre>
 *
 * MAX VOLUME is a integer in ASCII representation.
 *
 * The HTTP response includes the X-PourLogic-Auth header to verify
 * that the response is from the previous request. The HMAC provided
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
	
    // Verify HMAC
    Sha256.initHmac(key(), keySize());
    Sha256.println((unsigned long) _otp.count());      // server should have the same count (not provided in response)
    Sha256.println(HTTP_STATUS_OK);
    Sha256.println(maxVolume);
	
    return bytesToHexString(Sha256.resultHmac(), keySize()).equalsIgnoreCase(messageHmac);
}

/*! A pour result is an HTTP POST request with the following parameters:
 *   - user's RFID tag data (u)
 *   - the volume of the pour (v)
 *
 * The HTTP request must also include the X-PourLogic-Auth header in the
 * following format:
 *
 *   X-PourLogic-Auth: ID:COUNTER:HMAC
 *
 * where ID is the client's id, COUNTER is the OTP counter, and HMAC is
 * the result of hashing the request data in the following form:
 *
 * <pre>
 *   COUNTER\n
 *   REQUEST LINE\n
 *   [MESSAGE BODY]
 * </pre>
 *
 * MESSAGE BODY may be empty. If it is empty, then REQUEST LINE should
 * still end in a newline.
 *
 */
boolean PourLogicClient::sendPourResult(String const &tagData, float pourVolume)
{
    int content_length = 0;
    
    // Initialize HMAC
    initializeAuthHMAC();
    
    // -- Feed HMAC digest --
    content_length += Sha256.println(_otp.count());
    content_length += sendStatusLineHeadGet(&Sha256);
    content_length += sendPourResultStatusLine(&Sha256);
    content_length += sendStatusLineTail(&Sha256);
    content_length += Sha256.println();
    content_length += sendPourResultBody(tagData, pourVolume, &Sha256);
    // -- done HMAC
    
    // Send request to server --
    // Status/Request line
    sendStatusLineHeadGet(this);
    sendPourResultStatusLine(this);
    sendStatusLineTail(this);
    sendHTTPEndline(this);
    
    // HTTP headers
    sendHostHeader();
    sendUserAgentHeader();
    sendXPourLogicAuthHeader(id(), (unsigned long) _otp.count(), bytesToHexString(Sha256.resultHmac(), keySize()));
    sendContentLengthHeader(content_length); // includes sendHTTPEndline
    sendHTTPEndline(this);
    
    // Message Body
    sendPourResultBody(tagData, pourVolume, this);
    // -- done sending request 
    
    return true;
} 

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
 */
boolean PourLogicClient::parseXPourLogicAuthHeader(String const &line, String &hmac_result) {
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
      parseXPourLogicAuthHeader(line, hmac);
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

