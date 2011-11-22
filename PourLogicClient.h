#ifndef POURLOGIC_CLIENT_H
#define POURLOGIC_CLIENT_H

#include "config.h"

// Interface-specific includes
#include <WProgram.h>
#include <String.h>
#include <Ethernet.h>

#include "BasicOO.h"
#include "IPUtil.h"
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
class PourLogicClient : public Client
{
private:
  byte _key[32]; //!< HMAC secret key
  
  bool readHTTPLine(String &line);
  bool readHTTPLine(String &line, int maximumBytes);

  String getOTPCounter();  //!< Get the value of the OTP counter as a string.
  void incrementOTPCounter(); //!< Increment the value of the OTP counter.
  String getOTPHeaderHmac(String const &message); //!< Grab the HMAC from an X-Otp-Auth header line.
  
  const byte* key() { return _key; };
  uint8_t keySize() { return 32; /* for SHA256 */ };
  
  unsigned long id() { return (unsigned long) SETTINGS_CLIENT_ID; }
  const char* serverHostname() { return SETTINGS_SERVER_HOSTNAME; }
  const char* requestUri() { return SETTINGS_SERVER_REQUEST_URI; }
  const char* resultUri() { return SETTINGS_SERVER_RESULT_URI; }
  
protected:
  OTP _otp;

public:
  PourLogicClient(const ip4 &serverIP, uint16_t serverPort);
  ~PourLogicClient();
  
  bool sendPourRequest(String const &tagData); //!< Ask the server for permission to pour
  bool getPourRequestResponse(uint32_t &maxVolume); //!< Get the response for a pour request
  bool sendPourResult(String const &tagData, const uint32_t pourVolume); //!< Inform the server of a pour
  bool getPourResultResponse(); //!< Get the response to informing the server of a pour
};

#endif // #ifndef KEGBOT_CLIENT_H
