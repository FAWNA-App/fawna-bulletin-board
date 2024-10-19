#include <Arduino.h>
#include <NimBLEDevice.h>
#include <ArduinoJson.h>
#include <vector>

// UUIDs for BLE Service and Characteristics
#define SERVICE_UUID        "00005678-0000-1000-BEEF-00805F9B34FC"  // Bulletin Board Service UUID
#define POST_UUID           "00001234-0000-1000-BEEF-00805F9B34FC"     // Post Message Characteristic UUID
#define READ_UUID           "00005678-0000-1000-BEEF-00805F9B34FC"     // Read Messages Characteristic UUID

// Struct to represent a message
struct Message {
    char uuid[37];     // UUID should fit within 36 characters + null terminator
    char body[128];    // Message body
    int ttl;           // Time to live
};

// Global variables
std::vector<Message> messages;  // Using vector instead of map for more efficient memory usage
std::list<String> processedUUIDs;   // Store processed UUIDs for message relay
const int maxMessages = 10;      // Limit to prevent excessive memory use

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
        StaticJsonDocument<256> doc;  // Keep buffer small and static
        DeserializationError error = deserializeJson(doc, value);

        if (error) {
            Serial.print("Failed to parse JSON: ");
            Serial.println(error.f_str());
            return;
        }

        const char* uuid = doc["uuid"];
        const char* messageBody = doc["message"];
        int ttl = doc["ttl"];

        // Store the message if it has a valid TTL
        if (ttl > 0 && messages.size() < maxMessages) {  // Check for size limit
            Message msg;
            strncpy(msg.uuid, uuid, sizeof(msg.uuid));
            strncpy(msg.body, messageBody, sizeof(msg.body));
            msg.ttl = ttl;
            messages.push_back(msg);  // Use vector to store messages
            Serial.println("Message stored.");
        } else {
            Serial.println("Message not stored: TTL is 0 or message limit reached.");
        }
    }
};

class ReadMessageCallbacks : public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic) {
        String response = "";
        for (const auto& msg : messages) {
            response += "UUID: " + String(msg.uuid) + " Message: " + String(msg.body) + "\n";
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