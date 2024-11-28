#ifndef LEDSCONTROLLER_H
#define LEDSCONTROLLER_H

#include <Arduino.h>

class LedsController {
public:

public:
    LedsController();
    
    void setConnection(String connection_state);

    void blinkConnection();

    void show();

private:

    String connection_state;
};

#endif
