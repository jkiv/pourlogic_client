# PourLogic Client (Arduino v1.0 Edition)

## About

The PourLogic client controls access to a keg. The client relies on a server.
The server is responsible for deciding whether a given patron should be
served. The client either allows or disallows access to the keg based on
the server's decision. If a patron is allowed to pour, the server can
provide a maximum volume. The client will report the served volume to the
server when the patron completes his/her pour.

## Hardware Configuration

The PourLogic client interfaces with a number of pieces of hardware.  The
current version is being developed using the following hardware:

 * Arduino Ethernet Shield
 * Parallax RFID Card Reader (serial)
 * Seeedstudio G1/2 Flow Sensor
 * ASCO 8256 Series Solenoid valve (12V, NSF, N.C.)
 
## Software Configuration

Configuration for a PourLogic client instance can be found in `config.h`.
(Re)Configuring requires a (re)compile/(re)flash of the Arduino.

Read the information in `config.h` for configuring your PourLogic client.

## Compiling/Flashing

As of now, compiling and flashing the Arduino can be done usin the Arduino IDE
(http://arduino.cc/en/Main/Software) and using the Arduino

## Third-Party Libraries

The PourLogic client is dependent on some third-party libraries.

In order to successfully compile, you will need to install these libraries.
Check the Arudino website for more information on how/where to install these
libraries:

 * Cryptosuite (https://github.com/Cathedrow/Cryptosuite/) 

## License

See LICENSE.txt for copyright information.

## Acknowledgements

 * Kegbot project (http://kegbot.org)
 * OhmBase Hackerspace (http://ohmbase.org) :
   * Cory Koski
   * Jordan Manary
   * Taylor Moore
   * Chris Riley
   * Kris Scott
   * Bob Naish
 * Pete Binsted
 * Ben Zaporzan
