// See LICENSE.txt for license details.

// NOTE -- a lot of this code is messy in order to save RAM and/or time.

#include "PourLogicClient.h"
#include "StreamUtil.h"
#include "HexString.h"

static const int MAX_LINE_SIZE = 256;       //!< Length limit to an HTTP line (in bytes)
static const char HTTP_ENDLINE[] = "\r\n";
static const char HTTP_STATUS_OK[] = "200";


PourLogicClient::PourLogicClient(unsigned long api_id, String const& api_private_key)
  : __id(api_id)
{
  // Initialize timeout
  setTimeout(5000);
  
  // Initialize key for use with HMAC
  Sha256.init();
  Sha256.print(api_private_key);
  memcpy(_effective_key, Sha256.result(), _keySize());
}

PourLogicClient::~PourLogicClient() {
  stop();
}

unsigned long PourLogicClient::_printXPourLogicAuthHeader(Print& target) {
  unsigned long bytes_sent = 0;

  bytes_sent += target.print(F(CLIENT_AUTH_HEADER_NAME ": "));
  bytes_sent += target.print(_id());
  bytes_sent += target.print(F(":"));
  bytes_sent += target.print(_nonce.count());
  bytes_sent += target.print(F(":"));
  bytes_sent += target.print(bytesToHexString(Sha256.resultHmac(), _keySize()));
  bytes_sent += printHTTPEndline(target);
  
  return bytes_sent;
}

unsigned long PourLogicClient::_printPourRequestStatusLine(Print &target, String const& rfid) {
  unsigned long bytes_sent = 0;

  bytes_sent += printStatusLineHeadGet(target);
  bytes_sent += target.print(F(SERVER_POUR_RESULT_URI "?" CLIENT_POUR_REQUEST_PARAM_RFID "="));
  bytes_sent += target.print(rfid); // TODO hash rfid?
  bytes_sent += printStatusLineTail(target);
  
  return bytes_sent;
}

unsigned long PourLogicClient::_printPourResultStatusLine(Print& target) {
  unsigned long bytes_sent = 0;
  
  bytes_sent += printStatusLineHeadGet(target);
  bytes_sent += target.print(F(SERVER_POUR_RESULT_URI));
  bytes_sent += printStatusLineTail(target);
  bytes_sent += printHTTPEndline(target);
  
  return bytes_sent;
}

unsigned long PourLogicClient::_printPourResultMessageBody(Print &target, String const& rfid, float volume_in_mL) {
  unsigned long bytes_sent = 0;
  
  bytes_sent += target.print(F(CLIENT_POUR_RESULT_PARAM_RFID "="));
  bytes_sent += target.print(rfid);
  bytes_sent += target.print(F("&" CLIENT_POUR_RESULT_PARAM_VOLUME "="));
  bytes_sent += target.print(volume_in_mL);
  
  return bytes_sent;
}

void PourLogicClient::_initializeAuth() {
  // Initialize OTP for this request
  // .. increment counter
  _nonce.increment();
  // .. initialize Sha*
  Sha256.initHmac(_key(), _keySize());
}

/*! A pour request is an HTTP GET request with the following parameters:
 *   - user's RFID tag data (u)
 *
 * The HTTP request must also include the X-PourLogic-Auth header in the
 * following format:
 *
 *   X-PourLogic-Auth: ID:NONCE:HMAC
 *
 * where ID is the client's id, NONCE is the OTP counter, and HMAC is
 * the result of hashing the request data in the following form:
 *
 * <pre>
 *   NONCE\n
 *   REQUEST LINE\n
 *   [MESSAGE BODY]
 * </pre>
 *
 * MESSAGE BODY may be empty. If it is empty, then REQUEST LINE should
 * still end in a newline.
 */
boolean PourLogicClient::_sendPourRequest(String const &tagData) {
  // Initialize HMAC
  _initializeAuth();
  
  // -- Feed HMAC digest --
  Sha256.print(_nonce.count());                // NONCE\n
  Sha256.print('\n');
  _printPourRequestStatusLine(Sha256, tagData); // REQUEST LINE\n
  Sha256.print('\n');
                                              // empty body
  // -- done "Feed HMAC digest"
  
  // -- Send request to server --
  
  // Status/Request line
  _printPourRequestStatusLine(*this, tagData);
  printHTTPEndline(*this);
  
  // HTTP headers
  printHostHeader(*this);
  printUserAgentHeader(*this);
  _printXPourLogicAuthHeader(*this);
  printHTTPEndline(*this);
  
  // Message Body
  // .. empty
  
  // -- done "Send request to server"
  
  return true;
}

/*! A pour result is an HTTP POST request with the following parameters:
 *   - user's RFID tag data (u)
 *   - the volume of the pour (v)
 *
 * The HTTP request must also include the X-PourLogic-Auth header in the
 * following format:
 *
 *   X-PourLogic-Auth: ID:NONCE:HMAC
 *
 * where ID is the client's id, NONCE is the OTP counter, and HMAC is
 * the result of hashing the request data in the following form:
 *
 * <pre>
 *   NONCE\n
 *   REQUEST LINE\n
 *   [MESSAGE BODY]
 * </pre>
 *
 * MESSAGE BODY may be empty. If it is empty, then REQUEST LINE should
 * still end in a newline.
 *
 */
boolean PourLogicClient::_sendPourResult(String const &tagData, float pourVolume)
{
  int content_length = 0; // Required for POST request
  
  // Initialize HMAC
  _initializeAuth();
  
  // -- Feed HMAC digest --
  content_length += Sha256.print(_nonce.count());
  content_length += Sha256.print('\n');
  content_length += _printPourResultStatusLine(Sha256);
  content_length += Sha256.print('\n');
  content_length += _printPourResultMessageBody(Sha256, tagData, pourVolume);
  // -- done HMAC
  
  // Send request to server --
  // Status/Request line
  _printPourResultStatusLine(*this);
  printHTTPEndline(*this);
  
  // HTTP headers
  printHostHeader(*this);
  printUserAgentHeader(*this);
  _printXPourLogicAuthHeader(*this);
  printContentLengthHeader(*this, content_length);
  printHTTPEndline(*this);
  
  // Message Body
  _printPourResultMessageBody(*this, tagData, pourVolume);
  // -- done sending request 
  
  return true;
} 

////////////////////////////////////////////////////////////////////////////////

/*! A pour request response includes a body in the following format:
 *  <pre>
 *    MAX VOLUME\n
 *  </pre>
 *
 * MAX VOLUME is a integer in ASCII representation.
 *
 * The HTTP response includes the X-PourLogic-Auth header to verify
 * that the response is from the server. The value of the X-PourLogic-Atuh
 * in responses differs from that of requests. The value of the header is 
 * just an HMAC made from the keyed hash of the following:
 *
 * <pre>
 *   NONCE\n
 *   STATUS\n
 *   RESPONSE BODY
 * </pre>
 */
boolean PourLogicClient::_getPourRequestResponse(int& maxVolume)
{ 
  String messageHmac;
  maxVolume = 0;
  
  // Handle status line
  if (!_checkResponseStatusLine("200")) {
    return false;
  }
	
  // Read headers
  _parseResponseHeaders(messageHmac);

  // Read message body
  // .. maxVolume
  maxVolume = parseInt();
	
  // Verify HMAC (server should have same count as us)
  Sha256.initHmac(_key(), _keySize());
  Sha256.print((unsigned long) _nonce.count()); // nonce
  Sha256.print('\n');
  Sha256.print(HTTP_STATUS_OK); // HTTP status
  Sha256.print('\n');
  Sha256.print(maxVolume); // message body

  return bytesToHexString(Sha256.resultHmac(), _keySize()).equalsIgnoreCase(messageHmac);
}

/*! A pour result response verifies that the pour was recorded by responding
 * with a "200 OK".
 *
 * The HTTP response includes the X-PourLogic-Auth header to verify
 * that the response is from the server. The value of the X-PourLogic-Atuh
 * in responses differs from that of requests. The value of the header is 
 * just an HMAC made from the keyed hash of the following:
 *
 * <pre>
 *   NONCE\n
 *   STATUS\n
 *   RESPONSE BODY
 * </pre>
 */
boolean PourLogicClient::_getPourResultResponse()
{ 
  String messageHmac;
  
  // Handle status line
  if (!_checkResponseStatusLine(HTTP_STATUS_OK)) {
    return false;
  }
	
  // Read headers
  _parseResponseHeaders(messageHmac);

  // Read message body
  //   (empty)
	
  // Verify HMAC (server should have same count as us)
  Sha256.initHmac(_key(), _keySize());
  Sha256.print((unsigned long) _nonce.count()); // nonce
  Sha256.print('\n');
  Sha256.print(HTTP_STATUS_OK); // HTTP status
  Sha256.print('\n');
  // empty message body

  return bytesToHexString(Sha256.resultHmac(), _keySize()).equalsIgnoreCase(messageHmac);
}

/*!
 * Grab the HMAC from an X-Otp-Auth header line
 */
boolean PourLogicClient::_parseXPourLogicAuthHeader(String const &line, String &hmac_result) {
  
  int otpHeaderStart = line.indexOf((char*)F(CLIENT_AUTH_HEADER_NAME)); // TODO test this
  int otpHeaderHmacEnd = line.indexOf(HTTP_ENDLINE, otpHeaderStart);
  int otpHeaderHmacStart = line.indexOf(':', otpHeaderStart);
  
  if (otpHeaderStart < 0 || otpHeaderHmacEnd < 0 || otpHeaderHmacStart < 0) {
    return false; // No header or HMAC
  }
  
  // Strip out all but the HMAC
  hmac_result = line.substring(otpHeaderHmacStart+1, otpHeaderHmacEnd);
  hmac_result.trim();
  
  return (hmac_result.length() == 2*_keySize());
}

boolean PourLogicClient::_checkResponseStatusLine(const char* expected_status) {
  String line;
  
  // Read status line
  if (!readHTTPLine(*this, line, MAX_LINE_SIZE)) {
    return false; // Could not read status line
  }
  
  return line.indexOf(expected_status) >= 0;
}

boolean PourLogicClient::_parseResponseHeaders(String &hmac) {
  String line;
  
  for(;;) {
    // Read the next HTTP line
    line = "";
    if (!readHTTPLine(*this, line, MAX_LINE_SIZE)) {
      return false;
    }

    // Check headers
    // .. quit on empty line (i.e. \r\n)
    if (line.startsWith(HTTP_ENDLINE)) {
      break;
    }
      
    // .. grab the response id, HMAC, and count
    if (line.startsWith((char*)F(CLIENT_AUTH_HEADER_NAME))) { // TODO test this
      _parseXPourLogicAuthHeader(line, hmac);
    }
  }
  
  return true;
}

//!< Request the max. volume for a pour for the user given by tagData.
boolean PourLogicClient::requestMaxVolume(String const& tagData, int& maxVolume_mL) {
  maxVolume_mL = 0;
  
  // Request pour
  return (!connect(SETTINGS_SERVER_IP, SETTINGS_SERVER_PORT) ||
          !_sendPourRequest(tagData)                         ||
          !_getPourRequestResponse(maxVolume_mL));
}

//!< Send the result of a pour to the server.
boolean PourLogicClient::reportPouredVolume(String const& tagData, int const& volume_mL) {
  // Send pour results
  return (!connect(SETTINGS_SERVER_IP, SETTINGS_SERVER_PORT) ||
          !_sendPourResult(tagData, volume_mL)               ||
          !_getPourResultResponse());
}

boolean PourLogicClient::_test_xauth() {
  // TODO
}
