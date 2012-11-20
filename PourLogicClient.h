#ifndef POURLOGIC_CLIENT_H
#define POURLOGIC_CLIENT_H

#include <Arduino.h>
#include <String.h>
#include <Ethernet.h>
#include <sha256.h>

#include "config.h"
#include "OTP.h"

#define CLIENT_POUR_REQUEST_PARAM_RFID "u"
#define CLIENT_POUR_RESULT_PARAM_RFID "u"
#define CLIENT_POUR_RESULT_PARAM_VOLUME "v"

#define CLIENT_USER_AGENT  "pourlogic/shield/1.0"       // Arduino Uno + shield
//#define CLIENT_USER_AGENT  "pourlogic/board/1.0"        // PourLogic all-in-one board
//#define CLIENT_USER_AGENT  "pourlogic/shield-mega/1.0"  // Arduino Mega + shield
//#define CLIENT_USER_AGENT  "pourlogic/shield-pi/1.0"    // RaspberryPi + shield

/*!
 * At this time there are two different requests:
 *   - The pre-pour request, and
 *   - The post-pour result
 *
 * Pre-pour request:
 *
 * The pre-pour request asks the server if the controller should
 * allow flow to be active provided a token or set of credentials
 * (e.g. RFID tag number). The pre-pour request is sent the server
 * and the server responds with either OK, and a set of constraints
 * (e.g. maximum flow volume allowed), or not OK.
 * 
 * The post-pour result is sent to the server once a pour has been
 * made.  The controller informs the server about the details of the
 * pour (e.g. total volume). The server either accepts the pour and
 * updates itself with the information, or refuses the pour.
 *
 * There is no well defined strategy for handling a server that
 * refuses a pour at this time.  Since the server allowed the pour
 * a well behaved server should allow a response to that pour.
 *
 * If a server did not grant permission for a user to pour,
 * a well behaved controller should not allow the pour.
 *
 * Thus, all the controller can do at this time is ready itself for
 * the next user.
 *
 * All messages should contain some method of authentication (e.g. an
 * HMAC of the data using a pre-shared key) and some method to prevent
 * replay attacks (e.g. the use of nonces or timestamping).
 *
 * In our implementation we use an one-time-password scheme that
 * uses a persistent monotonic counter and a pre-shared-key.
 * (see #OTP).
 *
 * \brief A Client with some convenience functions for our pourlogic application.
 */
class PourLogicClient : public EthernetClient {

 private:
  byte _key[32]; //!< HMAC (effective) secret key

  String getOTPCounter();  //!< Get the value of the OTP counter as a string.
  void incrementOTPCounter(); //!< Increment the value of the OTP counter.
  
  const byte* key() { return _key; };
  uint8_t keySize() { return 32; /* for SHA256 */ };
  
  unsigned long id() { return (unsigned long) SETTINGS_CLIENT_ID; }
  const char* serverHostname() { return SETTINGS_SERVER_HOSTNAME; }
  const char* requestUri() { return SETTINGS_SERVER_POUR_REQUEST_URI; }
  const char* resultUri() { return SETTINGS_SERVER_POUR_RESULT_URI; }
  
    //!< Reads an HTTP line.
  boolean readHTTPLine(String &line);

  //!< Reads an HTTP line or until `maximumBytes' bytes are read.
  boolean readHTTPLine(String &line, int maximumBytes);

  //!< Prints the start of an HTTP status line for a GET query. (Separated out since it is duplicated among requests.)
  unsigned long sendStatusLineHeadGet(Print *target);

  //!< Prints the start of an HTTP status line for a POST query. (Separated out since it is duplicated among requests.)
  unsigned long sendStatusLineHeadPost(Print *target);

  //!< Prints the ending of an HTTP status line. (Separated out since it is duplicated among requests.)
  unsigned long sendStatusLineTail(Print *target);

  //!< Prints the ending of an HTTP line. (Separated out since it is duplicated among requests.)
  unsigned long sendHTTPEndline(Print *target);

  //!< Sends an HTTP request line for a "pour request" query
  unsigned long sendPourRequestStatusLine(String const& rfid, Print *target);

  //!< Sends an HTTP request line for a "pour result" query
  unsigned long sendPourResultStatusLine(Print *target);

  //!< Sends a Host header
  void sendHostHeader();

  //!< Sends a User-Agent header for the PourLogic client
  void sendUserAgentHeader();

  //!< Sends a Content-Type header for POST queries
  void sendContentTypePostHeader();

  //!< Sends a Content-Length header
  void sendContentLengthHeader(unsigned long content_length);

  //!< Sends an X-PourLogic-Auth header
  void sendXPourLogicAuthHeader(int bot_id, unsigned long counter, String const& HMAC);

  //!< Initializes HMAC for client-server authentication.
  void initializeAuthHMAC();

  //!< Grab the HMAC from an X-Otp-Auth header line
  boolean parseXPourLogicAuthHeader(String const &line, String &hmac_result);

  //!< Parses HTTP server response line and returns whether it contains an expected status.
  boolean checkResponseStatusLine(const char* expected_status);

  //!< Parses HTTP headers from server response. Keeps HMAC from X-PourLogic-Auth header.
  boolean parseResponseHeaders(String &hmac);

  boolean sendPourRequest(String const &tagData);
  boolean getPourRequestResponse(int& maxVolume);
  boolean sendPourResult(String const &tagData, float pourVolume);
  boolean getPourResultResponse();

 protected:
  OTP _otp;

 public:
  PourLogicClient(String const &key);
  ~PourLogicClient();
  
  //!< Request the max. volume for a pour for the user given by tagData.
  boolean requestMaxVolume(String const& tagData, int& maxVolume_mL);

  //!< Send the result of a pour to the server.
  boolean reportPouredVolume(String const& tagData, int const& volume_mL);
  
};

#endif // #ifndef POURLOGIC_CLIENT_H
