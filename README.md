This is the code for the electronic bulletin board named FAWNA, which uses an ESP32 microcontroller implementing Bluetooth Low-Energy (BLE) to connect to an Android device using the [FAWNA app](https://github.com/FAWNA-App/Fawna-App). The ESP32-powered FAWNA hub uses Universal Unique Identifiers (UUIDs) to read messages being sent from an Android device to the FAWNA hub to write/post those messages to the e-bulletin board hosted on the hub.

# Development Environment

* IDE: PlatformIO v3.3.3
* Framework: Arduino
* Libraries:
  * NimBLE (Bluetooth Low-Energy)
  * ArduinoJson
* Hardware
  * ESP32-3S-DevKitC-1
 
* Language: C++ 

# Useful Websites

* [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino)
* [PlatformIO Official Doc - NimBLE](https://registry.platformio.org/libraries/h2zero/NimBLE-Arduino)

# Future Work

* Implement a Bluetooth mesh network with multiple ESP32-powered FAWNA hubs and Android devices
* The e-bulletin board is currently being hosted on the Arduino devices using the FAWNA app and needs to be switched over to the ESP32-powered FAWNA hub
