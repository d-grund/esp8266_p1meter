#include "settings.h"
// **********************************
// * Ticker (System LED Blinker)    *
// **********************************
// * Initiate led blinker library
Ticker ticker;
// * Blink on-board Led
void tick()
{
    // * Toggle state
    int state = digitalRead(LED_BUILTIN);    // * Get the current state of GPIO1 pin
    digitalWrite(LED_BUILTIN, !state);       // * Set pin to the opposite state
}