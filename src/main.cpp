#include "classes/communication/CommunicationController.h"
#include "classes/leds/LedsController.h"
#include <esp_now.h>


CommunicationController communicationController;
LedsController ledsController;

void espIncomingData(const uint8_t* mac, const uint8_t* incomingData, int len) {
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    String message = String((char*)incomingData).substring(0, len);
    String node_mac(macStr);

    Serial.print("ESP_NOW message: ");
    Serial.println(message);

    communicationController.setIncomingData(node_mac, message);
    ledsController.blinkConnection();
}

void setup() {
  Serial.begin(115200);
  ledsController.show();
  communicationController.begin();

  if (esp_now_init() != ESP_OK) {
      Serial.println("Error initializing ESP-NOW");
      return;
  }
  esp_now_register_recv_cb(espIncomingData);

}

void loop() {
  String connection_state = communicationController.run();
  ledsController.setConnection( connection_state );

  delay(500);
}
