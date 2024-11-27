#include "CommunicationController.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
#include <esp_now.h>

#define BUTTON_PIN 12 // Define the button pin

// Constructor
CommunicationController::CommunicationController() {
    state = "IDLE";
}

// Initialization
void CommunicationController::begin() {
    pinMode(BUTTON_PIN, INPUT_PULLUP); // Set up the button pin

    // Initialize Wi-Fi in AP+STA mode for flexibility
    WiFi.mode(WIFI_AP_STA);
    wifiManager.setAPCallback(CommunicationController::configCallback);

    // Try to connect to previously saved Wi-Fi
    if (wifiManager.autoConnect("Environmental Node HUB Config", password)) {
        state = "CONNECTED";
        Serial.println("Connected to Wi-Fi!");
        Serial.print("Local IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        // If connection fails, stay in configuration mode
        Serial.println("Failed to connect. Configuration portal started.");
        state = "CONFIG_PORTAL";
        while (true) {
            delay(1000); // Stay in the config portal until connection succeeds
        }
    }

    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    esp_now_register_recv_cb(CommunicationController::onDataReceived);
}

// Main loop logic
String CommunicationController::run() {
    static bool lastButtonState = HIGH;
    bool currentButtonState = digitalRead(BUTTON_PIN);

    if (currentButtonState == LOW && lastButtonState == HIGH) {
        // Button pressed; toggle state
        if (state == "HOTSPOT") {
            stopAP();       // Stop hotspot
            startConnected(); // Switch to CONNECTED mode
        } else {
            stopConnected(); // Stop CONNECTED mode
            startAP();       // Switch to HOTSPOT mode
        }
    }

    lastButtonState = currentButtonState; // Update button state
    return state;
}

// Start hotspot mode
void CommunicationController::startAP() {
    if (WiFi.softAP(SSID, password)) {
        Serial.println("HOTSPOT mode enabled");
        Serial.print("Hotspot IP Address: ");
        Serial.println(WiFi.softAPIP());
        state = "HOTSPOT";
    } else {
        Serial.println("Failed to start Hotspot.");
    }
}

// Stop hotspot mode
void CommunicationController::stopAP() {
    WiFi.softAPdisconnect(true);
    Serial.println("HOTSPOT mode disabled");
}

// Start connected mode
void CommunicationController::startConnected() {
    // Ensure ESP32 remains in AP+STA mode to allow ESP-NOW communication
    WiFi.mode(WIFI_AP_STA);

    // Reconnect to Wi-Fi if needed
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Reconnecting to Wi-Fi...");
        WiFi.begin(); // Auto-connect to the last known network
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }
        Serial.println("Reconnected to Wi-Fi.");
    }

    Serial.println("CONNECTED mode enabled");
    Serial.print("Station IP Address: ");
    Serial.println(WiFi.localIP());
    state = "CONNECTED";
}

// Stop connected mode
void CommunicationController::stopConnected() {
    Serial.println("Stopped CONNECTED mode");
}

// ESP-NOW receive callback
void CommunicationController::onDataReceived(const uint8_t* mac, const uint8_t* incomingData, int len) {
    Serial.print("ESP-NOW Message Received from MAC: ");
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    Serial.println(macStr);

    String message = String((char*)incomingData);
    Serial.print("Message: ");
    Serial.println(message);

    // Example: Send HTTP POST
    post(macStr, message);
}

// Send HTTP POST to external server
void CommunicationController::post(const String& macAddress, const String& message) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wi-Fi not connected. Cannot send data.");
        return;
    }

    // HTTPClient http;
    // String serverUrl = "https://environmental-nodes.meandthebois.com/api/node/subdmi";

    // http.begin(serverUrl);
    // http.addHeader("Content-Type", "application/json");

    // String payload = "{\"macAddress\":\"" + macAddress + "\",\"message\":\"" + message + "\"}";
    // Serial.print("Payload: ");
    // Serial.println(payload);

    // int httpResponseCode = http.POST(payload);
    // if (httpResponseCode > 0) {
    //     Serial.print("HTTP Response Code: ");
    //     Serial.println(httpResponseCode);
    // } else {
    //     Serial.print("Error in HTTP request: ");
    //     Serial.println(http.errorToString(httpResponseCode));
    // }

    // http.end();
}

// Wi-FiManager configuration callback
void CommunicationController::configCallback(WiFiManager* manager) {
    Serial.println("Entered configuration mode");
    Serial.print("Config Portal IP Address: ");
    Serial.println(WiFi.softAPIP());
    Serial.print("Portal SSID: ");
    Serial.println(manager->getConfigPortalSSID());
}
