#include <Arduino.h>
#include <NimBLEDevice.h>
#include <ArduinoJson.h>

// UUIDs for BLE Service and Characteristics
#define SERVICE_UUID        "00000001-0000-1000-BEEF-00805F9B34FC"  // Bulletin Board Service UUID
#define WRITE_UUID           "00000002-0000-1000-BEEF-00805F9B34FC"     // Write Message Characteristic UUID
#define READ_UUID           "00000003-0000-1000-BEEF-00805F9B34FC"     // Read Messages Characteristic UUID

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
        
        // Send the stored messages to the client when connected
        String response = "";
        for (const auto& msg : messages) {
            response += "UUID: " + String(msg.uuid) + " Message: " + String(msg.body) + "\n";
        }
        // Assuming the characteristic to write messages back is defined; this might vary
        // Example: Send response back through a characteristic
        // pCharacteristic->setValue(response.c_str()); // Implement the right characteristic

        Serial.println("Sending messages to client upon connection.");
    }

    void onDisconnect(NimBLEServer* pServer) {
        Serial.println("Client disconnected, restarting advertising.");
        NimBLEDevice::startAdvertising();
    }
};

// Characteristic callbacks for handling message interactions
class WriteMessageCallbacks : public NimBLECharacteristicCallbacks {
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

void printStoredMessages() {
    Serial.println("Stored Messages:");
    
    if (messages.empty()) {
        Serial.println("No messages stored.");
        return;
    }
    
    for (const auto& msg : messages) {
        Serial.println("------------------------");
        Serial.print("UUID: ");
        Serial.println(msg.uuid);
        Serial.print("Message: ");
        Serial.println(msg.body);
        Serial.print("TTL: ");
        Serial.println(msg.ttl);
        Serial.println("------------------------");
    }
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

    /*---------------Store Demo Message-----------------------*/
    Message demoMessage;
    strncpy(demoMessage.uuid, WRITE_UUID, sizeof(demoMessage.uuid));
    strncpy(demoMessage.body, "Welcome to the Bulletin Board! :P", sizeof(demoMessage.body));
    demoMessage.ttl = 5;  // Example TTL
    messages.push_back(demoMessage);  // Add the demo message to the vector
    Serial.println("Demo message stored.");

    // Print stored messages
    printStoredMessages();
    /*--------------------------------------------------------*/

    // Create Write Message characteristic
    NimBLECharacteristic* pWriteCharacteristic = pService->createCharacteristic(
                                            WRITE_UUID,
                                            NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
                                        );
    pWriteCharacteristic->setCallbacks(new WriteMessageCallbacks());

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