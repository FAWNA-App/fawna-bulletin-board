#include <Arduino.h>
#include <NimBLEDevice.h>
#include <ArduinoJson.h>  // Include the Arduino JSON library
#include <map>
#include <list>

// UUIDs for BLE Service and Characteristics
#define SERVICE_UUID        "12345678-1234-5678-1234-56789abcdef0"   // Bulletin Board Service UUID
#define POST_UUID           "abcd1234-5678-1234-5678-abcdef123456"   // Post Message Characteristic UUID
#define READ_UUID           "abcd5678-1234-5678-1234-abcdef123456"   // Read Messages Characteristic UUID

// Struct to represent a message
struct Message {
    String uuid;
    String body;
    int ttl;
};

// Global variables
std::map<String, Message> messages; // Store posted messages
std::list<String> processedUUIDs;   // Store processed UUIDs for message relay

// BLE Server Callbacks for managing connection events
class MyServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        Serial.println("Client connected.");
    }

    void onDisconnect(NimBLEServer* pServer) {
        Serial.println("Client disconnected, restarting advertising.");
        NimBLEDevice::startAdvertising();
    }
};

// Characteristic callbacks for handling message interactions
class PostMessageCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) {
        String value = pCharacteristic->getValue().c_str();
        Serial.print("Received message: ");
        Serial.println(value);

        // Parse the incoming JSON message
        StaticJsonDocument<1024> doc; // Use StaticJsonDocument for a fixed size
        DeserializationError error = deserializeJson(doc, value);

        if (error) {
            Serial.print("Failed to parse JSON: ");
            Serial.println(error.f_str());
            return;
        }

        String uuid = doc["uuid"];
        String messageBody = doc["message"];
        int ttl = doc["ttl"];

        // Store the message if it has a valid TTL
        if (ttl > 0) {
            messages[uuid] = {uuid, messageBody, ttl};
            Serial.println("Message stored.");
        }
    }
};

class ReadMessageCallbacks : public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic) {
        String response = "";
        for (const auto& pair : messages) {
            response += "UUID: " + pair.second.uuid + " Message: " + pair.second.body + "\n";
        }
        pCharacteristic->setValue(response.c_str());
        Serial.println("Sending messages to client.");
    }
};

// Function to relay messages to nearby devices
void relayMessage(const String& uuid, const String& body, int ttl) {
    if (ttl <= 0) {
        return; // Stop if TTL is 0
    }

    // Check if UUID has been processed
    for (const auto& processedUUID : processedUUIDs) {
        if (processedUUID == uuid) {
            return; // Prevent loops
        }
    }
    
    // Store the UUID to avoid loops
    processedUUIDs.push_back(uuid);

    // Prepare message JSON
    StaticJsonDocument<1024> doc; // Use StaticJsonDocument for a fixed size
    doc["uuid"] = uuid;
    doc["message"] = body;
    doc["ttl"] = ttl - 1; // Decrement TTL

    String jsonString;
    serializeJson(doc, jsonString);
    Serial.println("Relaying message: " + jsonString);

    // Here you would implement BLE scanning and sending to other devices
    // This part needs to be implemented based on your Android device handling
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting BLE...");

    // Initialize BLE Device
    NimBLEDevice::init("ESP32_S3_NimBLE");   // Name of your BLE device

    // Create a BLE server
    NimBLEServer* pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create a BLE service
    NimBLEService* pService = pServer->createService(SERVICE_UUID);

    // Create Post Message characteristic
    NimBLECharacteristic* pPostCharacteristic = pService->createCharacteristic(
                                            POST_UUID,
                                            NIMBLE_PROPERTY::WRITE
                                        );
    pPostCharacteristic->setCallbacks(new PostMessageCallbacks());

    // Create Read Messages characteristic
    NimBLECharacteristic* pReadCharacteristic = pService->createCharacteristic(
                                            READ_UUID,
                                            NIMBLE_PROPERTY::READ
                                        );
    pReadCharacteristic->setCallbacks(new ReadMessageCallbacks());

    // Start the service
    pService->start();

    // Start advertising
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->start();
    Serial.println("Advertising started...");
}

void loop() {
    // Implement message relay logic here if needed
    // For example, periodically check for incoming messages and relay them
    delay(1000);
}
