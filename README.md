# PourLogic Client

## About

![Project status: hiatus](https://img.shields.io/badge/project status-abandoned-red.svg)

See the [Kegbot](http://kegbot.org) for an active alternative.

The PourLogic client controls access to a keg. The client relies on a server
(e.g. https://github.com/jkiv/pourlogic_server). The server is responsible for
deciding whether a given patron should be served. The client either allows or
disallows access to the keg based on the server's decision. If a patron is
allowed to pour, the server can provide a maximum volume. The server is also
useful for recording pour data. When a pour has taken place, the client will
report the pour details, such as the served volume, to the server.

## Hardware Configuration

The PourLogic client interfaces with a number of pieces of hardware.  The
current version is being developed using the following hardware:

 * PourLogic Arduino Shield (https://github.com/jkiv/pourlogic_board/)
 * Arduino Ethernet Shield
 * Parallax RFID Card Reader (serial)
 * Seeedstudio G1/2 Flow Sensor
 * ASCO 8256 Series Solenoid valve (12V, NSF, N.C.)
 
## Software Configuration

Configuration for a PourLogic client instance can be found in `config.h`.
(Re)Configuring requires a (re)compile/(re)flash of the Arduino.

Read the information in `config.h` for configuring your PourLogic client.

## Compiling/Flashing

As of now, compiling and flashing the Arduino can be done using the Arduino IDE
(http://arduino.cc/en/Main/Software).

## Third-Party Libraries

The PourLogic client is dependent on some third-party libraries. In order to
successfully compile, you will need to install these libraries. Check the
Arudino website for more information on how/where to install these libraries:

 * Cryptosuite (https://github.com/jkiv/Cryptosuite/) 

## License

See LICENSE.txt for copyright information.

## Acknowledgements

 * Kegbot (http://kegbot.org) for the inspiration
 * OhmBase Hackerspace (http://ohmbase.org) :
   * Cory Koski
   * Jordan Manary
   * Taylor Moore
   * Chris Riley
   * Kris Scott
   * Bob Naish
 * Pete Binsted
 * Ben Zaporzan
