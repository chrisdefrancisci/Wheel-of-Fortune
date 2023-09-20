// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef SEQUENCER_H_
#define SEQUENCER_H_

//add your includes for the project Sequencer here

#include "Arduino.h"

// My header files
#include "CommStatus.h"
#include "SequencerDriver.h"
#include "AD5695.h"
#include "Display.h"
//#include "AT42_QT1245_Touch_driver.h"
#include "CapacitiveButtons.h"
#include "SPI.h"
#include "StateMachine.h"
#include "Hardware.h"


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
// TODO: move to Atmel_AT43-QT1245 files
void nCHANGE_ISR()
{
  new_touch = true;
  detachInterrupt(digitalPinToInterrupt(NCHANGE_PIN));
}

//Do not add code below this line
#endif /* SEQUENCER_H_ */
