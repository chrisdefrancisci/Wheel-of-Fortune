// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _Sequencer_H_
#define _Sequencer_H_
#include "Arduino.h"
//add your includes for the project Sequencer here
#include "SPI.h"
//#include "Atmel_AT42-QT1245.h"
#include "IS31FL3246_LED_driver.h"


//end of add your includes here

// Global variables
volatile byte new_touch = false;
byte data_read = false;

//add your function definitions for the project Sequencer here


// The nCHANGE pin can be used to alert the host to key touches or key releases, thus reducing
// the need for unnecessary communications. Normally, the host can simply not bother to communicate
// with the device, except when the CHANGE pin becomes active.
//
// Multiple devices can be connected together in a single wire-OR logic connection with the host.
// When the CHANGE pin goes active, the host can poll all devices to identify which one is reporting
// a touch change.
//void nCHANGE_ISR()
//{
//  new_touch = true;
//  detachInterrupt(digitalPinToInterrupt(nCHANGE_pin));
//}

//Do not add code below this line
#endif /* _Sequencer_H_ */
