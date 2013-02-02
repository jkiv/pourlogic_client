// See LICENSE.txt for license details.

#include "PourLogicClient.h"
#include "StreamUtil.h"
#include "HexString.h"
#include "HTTPUtil.h"

#define MAX_LINE_SIZE 256       //!< Length limit to an HTTP line (in bytes)
static const char HTTP_ENDLINE[] = "\r\n";
static const char HTTP_STATUS_OK[] = "200";
static char line_buffer[MAX_LINE_SIZE];

PourLogicClient::PourLogicClient(unsigned long api_id, const char* api_private_key)
  : __id(api_id)
{
  // Initialize timeout
  setTimeout(2000);
  
  // Initialize nonce
  _nonce.begin();
  
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
  bytes_sent += target.print(SERVER_POUR_REQUEST_URI "?" CLIENT_POUR_REQUEST_PARAM_RFID "=");
  bytes_sent += target.print(rfid); // TODO hash rfid?
  bytes_sent += printStatusLineTail(target);
  //bytes_sent += printHTTPEndline(target);
  
  return bytes_sent;
}

unsigned long PourLogicClient::_printPourResultStatusLine(Print& target) {
  unsigned long bytes_sent = 0;
  
  bytes_sent += printStatusLineHeadPost(target);
  bytes_sent += target.print(SERVER_POUR_RESULT_URI);
  bytes_sent += printStatusLineTail(target);
  //bytes_sent += printHTTPEndline(target);
  
  return bytes_sent;
}

unsigned long PourLogicClient::_printPourResultMessageBody(Print &target, String const& rfid, float volume_in_mL) {
  unsigned long bytes_sent = 0;
  
  bytes_sent += target.print(CLIENT_POUR_RESULT_PARAM_RFID "=");
  bytes_sent += target.print(rfid);
  bytes_sent += target.print("&" CLIENT_POUR_RESULT_PARAM_VOLUME "=");
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
 * The HTTP request must also include the X-Pourlogic-Auth header in the
 * following format:
 *
 *   X-Pourlogic-Auth: ID:NONCE:HMAC
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
  Sha256.print(_nonce.count());                 // NONCE\n
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
  printContentLengthHeader(*this, 0);
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
 * The HTTP request must also include the X-Pourlogic-Auth header in the
 * following format:
 *
 *   X-Pourlogic-Auth: ID:NONCE:HMAC
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
boolean PourLogicClient::_sendPourResult(String const &tag_data, float pour_volume) {
  int content_length = 0; // Required for POST request
  
  // Initialize HMAC
  _initializeAuth();
  
  // -- Feed HMAC digest --
  Sha256.print(_nonce.count());
  Sha256.print('\n');
  _printPourResultStatusLine(Sha256);
  Sha256.print('\n');
  content_length += _printPourResultMessageBody(Sha256, tag_data, pour_volume);
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
  _printPourResultMessageBody(*this, tag_data, pour_volume);
  
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
 * The HTTP response includes the X-Pourlogic-Auth header to verify
 * that the response is from the server. The value of the X-Pourlogic-Atuh
 * in responses differs from that of requests. The value of the header is 
 * just an HMAC made from the keyed hash of the following:
 *
 * <pre>
 *   NONCE\n
 *   STATUS\n
 *   RESPONSE BODY
 * </pre>
 */
boolean PourLogicClient::_getPourRequestResponse(int& max_volume_mL)
{ 
  String message_hmac;
  char our_hmac[65];
  max_volume_mL = 0;
  our_hmac[64]='\0';
  
  // Handle status line
  if (!_checkResponseStatusLine(HTTP_STATUS_OK)) {
    return false;
  }
  
  // Read headers
  if (!_parseResponseHeaders(message_hmac)) {
    return false;
  }
  
  // Read message body
  // .. maxVolume
  max_volume_mL = parseInt();

  // Verify HMAC (server should have same count as us)
  Sha256.initHmac(_key(), _keySize());
  Sha256.print(_nonce.count()); // nonce
  Sha256.print('\n');
  Sha256.print(HTTP_STATUS_OK); // HTTP status
  Sha256.print('\n');
  Sha256.print(max_volume_mL); // message body
  
  bytesToHexString(our_hmac, Sha256.resultHmac(), _keySize());
 
  return message_hmac.equalsIgnoreCase(our_hmac);
}

/*! A pour result response verifies that the pour was recorded by responding
 * with a "200 OK".
 *
 * The HTTP response includes the X-Pourlogic-Auth header to verify
 * that the response is from the server. The value of the X-Pourlogic-Atuh
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
  if (!_parseResponseHeaders(messageHmac)) {
    return false;
  }

  // Read message body
  //   (empty)
	
  // Verify HMAC (server should have same count as us)
  Sha256.initHmac(_key(), _keySize());
  Sha256.print(_nonce.count()); // nonce
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
  
  int otpHeaderStart = line.indexOf(CLIENT_AUTH_HEADER_NAME); // TODO test this
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
  // Read status line
  if (!readHTTPLine(*this, MAX_LINE_SIZE, line_buffer)) {
    return false; // Could not read status line
  }
  
  return (NULL != strstr(line_buffer, expected_status));
}

boolean PourLogicClient::_parseResponseHeaders(String &hmac) {
  
  for(;;) {
    // Read the next HTTP line
    line_buffer[0] = '\0';
    if (!readHTTPLine(*this, MAX_LINE_SIZE, line_buffer)) {
      return false;
    }

    // Check headers
    // .. quit on empty line (i.e. \r\n)
    //if (line.startsWith(HTTP_ENDLINE)) {
    if (strstr(line_buffer, HTTP_ENDLINE) == line_buffer) { // TODO strBeginWith()?
      break;
    }
      
    // .. grab the response id, HMAC, and count
    if (strstr(line_buffer, CLIENT_AUTH_HEADER_NAME) == line_buffer) {
      _parseXPourLogicAuthHeader(line_buffer, hmac);
    }
  }
  
  return true;
}

//!< Request the max. volume for a pour for the user given by tagData.
boolean PourLogicClient::requestMaxVolume(String const& tag_data, int& max_volume_mL) {
  boolean success = false;
  
  // Request pour
  success = (connect(SETTINGS_SERVER_IP, SETTINGS_SERVER_PORT) &&
             _sendPourRequest(tag_data)                        &&
             _getPourRequestResponse(max_volume_mL));
  
  shutdown();
  return success;
}

//!< Send the result of a pour to the server.
boolean PourLogicClient::reportPouredVolume(String const& tag_data, float volume_mL) {
  boolean success = false;
  
  // Send pour results
  success = (connect(SETTINGS_SERVER_IP, SETTINGS_SERVER_PORT) &&
             _sendPourResult(tag_data, volume_mL)              &&
             _getPourResultResponse());

  shutdown();
  return success;
}

void PourLogicClient::shutdown() {
  stop();
  readUntilUnavailable(this); // flush receive buffer
}
