// See LICENSE.txt for license details.

#ifndef POURLOGIC_CLIENT_H
#define POURLOGIC_CLIENT_H

#include <Arduino.h>
#include <String.h>
#include <Ethernet.h>
#include <sha1.h>

#include "config.h"
#include "Nonce.h"

#define CLIENT_POUR_REQUEST_PARAM_RFID "u"
#define CLIENT_POUR_RESULT_PARAM_RFID "u"
#define CLIENT_POUR_RESULT_PARAM_VOLUME "v"

#define CLIENT_AUTH_HEADER_NAME "X-Pourlogic-Auth"

// NOTE: Rake/Rails cannot reconstruct our request URI exactly as sent, so we omit the trailing slash here to match
#define SERVER_POUR_REQUEST_URI "/pours/new" //!< URI to request when requesting to pour
#define SERVER_POUR_RESULT_URI "/pours"      //!< URI to request when sending result
#define SERVER_TEST_XAUTH_URI "/test/xauth"  //!< URI to test X-Pourlogic-Auth

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
 * (see #Nonce).
 *
 * \brief A Client with some convenience functions for our pourlogic application.
 */
class PourLogicClient : public EthernetClient {

 private:
  byte _effective_key[20]; //!< HMAC (effective) secret key
  unsigned long __id;
  
  const byte* _key() { return _effective_key; };
  int _keySize() { return 20; };
  unsigned long _id() { return __id; }
  
  //!< Initializes HMAC for client-server authentication.
  void _initializeAuth();
  
  //!< Called on communications failure. Disconnects and flushes receive buffer.
  boolean _failure();

  //!< Grab the HMAC from an X-Pourlogic-Auth header
  boolean _parseXPourLogicAuthHeader(String const &line, String &hmac_result);

  //!< Parses HTTP server response line and returns whether it contains an expected status.
  boolean _checkResponseStatusLine(const char* expected_status);

  //!< Parses HTTP headers from server response. Keeps HMAC from X-Pourlogic-Auth header.
  boolean _parseResponseHeaders(String &hmac);
  
  unsigned long _printXPourLogicAuthHeader(Print& target); //!< Write out the X-Pourlogic-Auth header and data (assuming ready)
  unsigned long _printPourRequestStatusLine(Print &target, String const& rfid); //!< Write the status line for a "pour request"
  unsigned long _printPourResultStatusLine(Print &target); //!< Write the status line for a "pour result"
  unsigned long _printPourResultMessageBody(Print &target, String const& rfid, float volume_in_mL); // Write the "pour result" message body
  
  // Request parts ////////////////////////////////////////////////////////////
  boolean _sendPourRequest(String const &tagData);
  boolean _getPourRequestResponse(int& max_volume);
  boolean _sendPourResult(String const &tag_data, float pour_volume);
  boolean _getPourResultResponse();
  
 protected:
  Nonce _nonce;

 public:
  PourLogicClient(unsigned long api_id, const char* api_private_key);
  ~PourLogicClient();
  
  //!< Close connection and flush the receive buffer
  void shutdown();
  
  //!< Request the max. volume for a pour for the user given by tagData.
  boolean requestMaxVolume(String const& tag_data, int& max_volume_mL);
  
  //!< Send the result of a pour to the server.
  boolean reportPouredVolume(String const& tag_data, float volume_mL);
};

#endif // #ifndef POURLOGIC_CLIENT_H
