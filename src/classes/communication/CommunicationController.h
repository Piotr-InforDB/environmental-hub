#ifndef COMMUNICATIONCONTROLLER_H
#define COMMUNICATIONCONTROLLER_H

#include <WiFi.h>
#include <Arduino.h>
#include <WiFiManager.h> 
#include <Preferences.h>

class CommunicationController {
public:
    CommunicationController();
    void begin();
    String run();
    void startAP();
    void stopAP();

    void startConnected();
    void stopConnected();

    void setIncomingData(String mac, String data);
    void postNodeData();

    static void configCallback(WiFiManager *manager);
    static void onDataReceived(const uint8_t *mac, const uint8_t *incomingData, int len);
    static void test();
    
private:
    WiFiManager wifiManager;
    String state;

    String inbound_mac;
    String inbound_data;

    const char* SSID = "Environmental Node HUB";
    const char* password = "staging_password_123";
};

#endif