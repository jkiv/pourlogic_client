#ifndef POURLOGIC_CLIENT_H
#define POURLOGIC_CLIENT_H

#include <Arduino.h>
#include <String.h>
#include <Ethernet.h>
#include <sha256.h>

#include "config.h"
#include "OTP.h"

/*!
 * At this time there are two different requests:
 *   - The pre-pour request, and
 *   - The post-pour result
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
  byte _key[32]; //!< HMAC secret key
  
  boolean readHTTPLine(String &line);
  boolean readHTTPLine(String &line, int maximumBytes);

  String getOTPCounter();  //!< Get the value of the OTP counter as a string.
  void incrementOTPCounter(); //!< Increment the value of the OTP counter.
  String getOTPHeaderHmac(String const &message); //!< Grab the HMAC from an X-Otp-Auth header line.
  
  const byte* key() { return _key; };
  uint8_t keySize() { return 32; /* for SHA256 */ };
  
  unsigned long id() { return (unsigned long) SETTINGS_CLIENT_ID; }
  const char* serverHostname() { return SETTINGS_SERVER_HOSTNAME; }
  const char* requestUri() { return SETTINGS_SERVER_POUR_REQUEST_URI; }
  const char* resultUri() { return SETTINGS_SERVER_POUR_RESULT_URI; }
  
  void signPourRequest(char* pgmbuffer, String const& tagData, String& hmac); //!< Creates an HMAC for a pour request
  boolean sendPourRequest(String const &tagData); //!< Ask the server for permission to pour
  boolean getPourRequestResponse(int &maxVolume); //!< Get the response for a pour request
  
  void signPourResult(char* pgmbuffer, String const& tagData, int const& volume_mL, String& hmac);
  boolean sendPourResult(String const &tagData, const int pourVolume); //!< Inform the server of a pour
  boolean getPourResultResponse(); //!< Get the response to informing the server of a pour
  
  boolean checkResponseStatusLine(const char* expectedStatus);
  boolean parseResponseHeaders(String& messageHmac);
  
  void printXOtpAuthHeader(char* pgmbuffer, int id, unsigned long counter, String const& hmac);
  boolean parseXOtpAuthHeader(String const& line, String& hmac_result);

 protected:
  OTP _otp;

 public:
  PourLogicClient(String const &key);
  ~PourLogicClient();

  boolean requestMaxVolume(String const& tagData, int& maxVolume_mL);
  boolean reportPouredVolume(String const& tagData, int const& volume_mL); //!< Send the result of a pour to the server.
};

#endif // #ifndef POURLOGIC_CLIENT_H
