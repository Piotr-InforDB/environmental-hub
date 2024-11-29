#include "CommunicationController.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
#include <esp_now.h>

#define BUTTON_PIN 12

CommunicationController::CommunicationController() {
    state = "CONFIG_PORTAL";
}

void CommunicationController::begin() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    WiFi.mode(WIFI_AP_STA);

    if (wifiManager.autoConnect("Environmental Node HUB Config", password)) {
        state = "CONNECTED";
        Serial.println("Connected to Wi-Fi!");
        Serial.print("Local IP Address: ");
        Serial.println(WiFi.localIP());
    }
}

String CommunicationController::run() {
    if(state == "CONFIG_PORTAL"){ return state; }

    bool current_button_state = digitalRead(BUTTON_PIN);
    if (current_button_state == LOW && last_button_state == HIGH) {
        if (state == "HOTSPOT") {
            stopAP();
            stopBLE();
            startConnected();
        } else {
            stopConnected();
            startAP();
            startBLE();
        }
    }

    postNodeData();

    last_button_state = current_button_state;
    return state;
}

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
void CommunicationController::stopAP() {
    WiFi.softAPdisconnect(true);
    Serial.println("HOTSPOT mode disabled");
}

void CommunicationController::startBLE() {
    NimBLEDevice::init("Environmental Node Hub");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

    pServer = NimBLEDevice::createServer();

    pService = pServer->createService(BLE_SERVICE_UUID);

    pCharacteristic = pService->createCharacteristic(
        BLE_CHARACTERISTICS_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );

    String macAddress = WiFi.macAddress();
    pCharacteristic->setValue(macAddress.c_str());

    pService->start();

    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->start();

    Serial.println("BLE started");
}
void CommunicationController::stopBLE() {
    NimBLEDevice::deinit(true);
    Serial.println("BLE stopped");
}

void CommunicationController::startConnected() {
    WiFi.mode(WIFI_AP_STA);

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Reconnecting to Wi-Fi...");
        WiFi.begin();
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
void CommunicationController::stopConnected() {
    Serial.println("Stopped CONNECTED mode");
}

void CommunicationController::setIncomingData(String mac, String data){
    inbound_mac[inbound_index] = mac;
    inbound_data[inbound_index] = data;

    inbound_index ++;
    if(inbound_index >= inbound_cap){ inbound_index = 0; }
}
void CommunicationController::postNodeData() {
    int index = -1;
    for(int i = 0; i < inbound_cap; i++){
        if(inbound_mac[i] != ""){
            index = i;
            break;
        }
    }
    if(index == -1){ return; }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wi-Fi not connected. Cannot send data.");
        return;
    }

    HTTPClient http;
    http.begin("https://environmental-nodes.meandthebois.com/api/node/submit");

    String payload = "{\"hub\":\"" + WiFi.macAddress() + "\", \"node\":\"" + inbound_mac[index] + "\",\"data\":" + inbound_data[index] + "}";
    Serial.print("Payload: ");
    Serial.println(payload);

    inbound_mac[index] = "";
    inbound_data[index] = "";

    int httpResponseCode = http.POST(payload);
    if (httpResponseCode > 0) {
        Serial.print("HTTP Response Code: ");
        Serial.println(httpResponseCode);
    } else {
        Serial.print("Error in HTTP request: ");
        Serial.println(http.errorToString(httpResponseCode));
    }

    http.end();
}
