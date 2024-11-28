#include "CommunicationController.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
#include <esp_now.h>

#define BUTTON_PIN 12

CommunicationController::CommunicationController() {
    state = "IDLE";
}

void CommunicationController::begin() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    WiFi.mode(WIFI_AP_STA);
    wifiManager.setAPCallback(CommunicationController::configCallback);

    if (wifiManager.autoConnect("Environmental Node HUB Config", password)) {
        state = "CONNECTED";
        Serial.println("Connected to Wi-Fi!");
        Serial.print("Local IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("Failed to connect. Configuration portal started.");
        state = "CONFIG_PORTAL";
        while (true) {
            delay(1000);
        }
    }
}

String CommunicationController::run() {
    static bool lastButtonState = HIGH;
    bool currentButtonState = digitalRead(BUTTON_PIN);

    if (currentButtonState == LOW && lastButtonState == HIGH) {
        if (state == "HOTSPOT") {
            stopAP();
            startConnected();
        } else {
            stopConnected();
            startAP();
        }
    }

    postNodeData();

    lastButtonState = currentButtonState;
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

void CommunicationController::configCallback(WiFiManager* manager) {
    Serial.println("Entered configuration mode");
    Serial.print("Config Portal IP Address: ");
    Serial.println(WiFi.softAPIP());
    Serial.print("Portal SSID: ");
    Serial.println(manager->getConfigPortalSSID());
}
