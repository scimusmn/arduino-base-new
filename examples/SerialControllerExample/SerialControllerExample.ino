#include <Arduino.h>

#include "SerialController.h"

SerialController serialController;

long baudrate = 115200;
int blinkrate = 1000;
bool blinking = true;
bool led_on = false;
long blinkcounter = 0;

#define ledpin 13

// a callback function that will be linked with the serial key "blinkrate"
void onBlinkrate(int rate) {
  blinkrate = rate;
}

// a callback function that will be linked with the serial key "blinking"
void onBlinking(int state) {
  blinking = state;
}


void setup() {
  // Ensure Serial Port is open and ready to communicate
  serialController.setup(baudrate);

  // register a callback function to the serial key "blinking"
  serialController.addCallback("blinking", onBlinking);
  // register a callback function to the serial key "blinkrate"
  serialController.addCallback("blinkrate", onBlinkrate);
  // register a lambda callback to the serial key "led"
  serialController.addCallback("led", [](int s){ digitalWrite(ledpin, s); });

  // For every sketch, we need to set up our IO
  // Setup digital pins and default modes as needed, analog inputs are setup by default
  pinMode(ledpin, OUTPUT);
}

void loop() {
  // update SerialController and check for new data
  serialController.update();

  if (blinking) {
    if ((millis() - blinkcounter) >= blinkrate) {
      led_on = !led_on;
      digitalWrite(ledpin,led_on);
      blinkcounter = millis();

      // send data to stele
      serialController.sendMessage("led-status", led_on);
    }
  }
}
