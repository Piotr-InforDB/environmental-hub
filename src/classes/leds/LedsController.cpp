#include "LedsController.h"
#include <FastLED.h>
#include <Arduino.h>

#define NUM_LEDS 1
#define DATA_PIN 13

CRGB leds[NUM_LEDS];

LedsController::LedsController(){
    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

    for(int i = 0; i < NUM_LEDS; i++){
        leds[i] = 0xFF0000;
    }

    this->connection_state = "CONFIG_PORTAL";
}


void LedsController::setConnection(String connection_state){
    this->connection_state = connection_state;
    this->show();
}

void LedsController::blinkConnection(){
    leds[0] = 0x000000;
    FastLED.show();

    delay(50);
    show();
}

void LedsController::show(){

    if(connection_state == "CONFIG_PORTAL"){
        leds[0] = CRGB(25, 0, 25);
    }
    else if(connection_state == "CONNECTED"){
        leds[0] = CRGB(0, 25, 0);
    }
    else if(connection_state == "HOTSPOT"){
        leds[0] = CRGB(0, 0, 25);
    }
    else{
        leds[0] = CRGB(0, 0, 0);
    }

    FastLED.show();
}