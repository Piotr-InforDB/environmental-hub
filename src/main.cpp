#include "classes/communication/CommunicationController.h"

CommunicationController communicationController;


void setup() {
  Serial.begin(115200);
  communicationController.begin();
}

void loop() {
  String state = communicationController.run();
  delay(500);
}
