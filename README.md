This is a networking demo made from connecting an ESP32 microcontroller to an Android phone using Bluetooth Low-Energy (BLE) and show the Universal Unique Identifiers (UUIDs) advertised by the the ESP32 to the Android phone using the BLE Simulator app from the F-Droid app store.

# Network Communication

This project uses peer-to-peer networking architecture using TCP.

# Development Environment

* IDE: PlatformIO v3.3.3
* Framework: Arduino
* Libraries:
  * NimBLE (Bluetooth Low-Energy)
  * ArduinoJson
* Hardware
  * ESP32-3S-DevKitC-1
 
* Language: Professional Business 

# Useful Websites

{Make a list of websites that you found helpful in this project}
* [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino)
* [PlatformIO Official Doc - NimBLE](https://registry.platformio.org/libraries/h2zero/NimBLE-Arduino)

# Future Work

{Make a list of things that you need to fix, improve, and add in the future.}
* I need to buy more ESP32s and build a Bluetooth mesh network with them
* The UUIDs need to have more uniqueness in their values
* I need to handle accepting messages
