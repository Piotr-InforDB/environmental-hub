#include <WiFi.h>

#define BUTTON_PIN 13
const char* ssid = "Environmental Node HUB";
const char* password = "staging_password_123";

bool apStarted = false;

void startAP() {
  if (WiFi.softAP(ssid, password)) {
    Serial.println("Access Point Started");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("Failed to start Access Point");
  }
}

void stopAP() {
  WiFi.softAPdisconnect(true);
  Serial.println("Access Point Stopped");
}


void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  WiFi.mode(WIFI_MODE_AP);
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    if (!apStarted) {
      startAP();
      apStarted = true;
    }
  } else {
    if (apStarted) {
      stopAP();
      apStarted = false;
    }
  }
}