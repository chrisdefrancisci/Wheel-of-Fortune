// Do not remove the include below
#include "Sequencer.h"

// Constants
const uint16_t recording_blink_time = 500; // Visual indicator for recording

// DAC
//const uint8_t dac_driver_address = B0001100;

// Other
// TODO find a better way to do pin assignments that doesn't involve converting to and from
// 		the arduino pin convention


// Global variables
Display DisplayDriver;
AD5695 DacDriver(DAC_DRIVER_ADDRESS);
SequencerDriver	SeqDriver(&DacDriver);
StateMachine StateManager;

//AT42_QT1245_Touch_driver TouchDriver(CHIP_SELECT_PIN, NCHANGE_PIN, NDRDY_PIN, ARDUINO_SELECT_PIN);
CapacitiveButtons TouchDriver(CHIP_SELECT_PIN, NCHANGE_PIN, NDRDY_PIN, ARDUINO_SELECT_PIN, &DisplayDriver);

//The setup function is called once at startup of the sketch
void setup()
{

	// Pin setups
	// Debug LED
	*pDebugLedSetup |= debugLed; // Set to output (1)
//	*pDebugLedPort |= debugLed; // Set high
	// Device peripherals
	*pDevResetSetup |= devReset; // Set to output (1)
	*pDevResetPort |= devReset; // Set high
	//Gate outputs
	*pGate0Setup |= gate0; // Set to output (1)
	*pGate0Port &= ~gate0; // Set low
	*pGate1Setup |= gate1; // Set to output (1)
	*pGate1Port &= ~gate1; // Set low
	*pGate2Setup |= gate2; // Set to output (1)
	*pGate2Port &= ~gate2; // Set low
	*pGate3Setup |= gate3; // Set to output (1)
	*pGate3Port &= ~gate3; // Set low

	// Communication setups

	Serial.begin(9600);
	Serial.println("Initializing...");

	DacDriver.begin();

	// Peripheral setups
	DisplayDriver.begin();

	while (!DisplayDriver.rainbowLoop(100)){

	}

	// Initialize device, set EEPROM if needed
	TouchDriver.begin();

	// Set interrupt for pin that signals change in touch
	pinMode(NCHANGE_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(NCHANGE_PIN), nCHANGE_ISR, LOW);

	Serial.println("Setup complete.");
	DisplayDriver.circleOff();
	DisplayDriver.peripheralOff();


	SeqDriver.begin();

}

// The loop function is called in an endless loop
void loop()
{
	/**
	 * Main loop.
	 */
	// Variables corresponding to sequencer management - TODO: perhaps this could be wrapped into
	// its own class
	static uint8_t active_sequencer = 0;
	static DacAddr active_dac = DacAddr::DACA;
	static rgb8_t active_color = sequencerColors[active_sequencer];

	// Current user input
	Bitfield<QT1245_DETECT_BYTES> allPressedKeys;
	Bitfield<QT1245_DETECT_BYTES> newPressedKeys;

	// Visual indicators
	static uint32_t last_blink_time = 0;

	Serial.flush();
	// transfer data from device
	if(new_touch)
	{
		Serial.print("New touch detected: ");
		// Key touch housekeeping
		new_touch = false;
		TouchDriver.updatePressedKeys();
		allPressedKeys = TouchDriver.getAllPressedKeys();
		newPressedKeys = TouchDriver.getNewPressedKeys(); // TODO: this might actually not be necessary
													 	  // 	Actions happen when key is first pressed
														  //	Although some debouncing may be necessary?


		if (newPressedKeys[BUTTON_PLAY]) {
			// Check keys for state transition
			if (allPressedKeys[BUTTON_FUNC]) {
				// Start or stop recording
				// TODO: this doesn't really seem to work
				if (StateManager.getState() == SequencerState::Stop){
					StateManager.setState(SequencerState::Record);
					Serial.println("Record state");
					SeqDriver.restartIndex();
				}
				else {
					StateManager.setState(SequencerState::Stop);
					Serial.println("Stop state");
				}
			}
			// If stopped, play. Otherwise, stop.
			else if (StateManager.getState() == SequencerState::Stop){
				SeqDriver.restartIndex();
				StateManager.setState(SequencerState::Play);
				Serial.println("Play state");
			}
			else {
				StateManager.setState(SequencerState::Stop);
				Serial.println("Stop state");
			}
		}
		else if (TouchDriver.getChangedNote() >= NOTE_i && TouchDriver.getChangedNote() <= NOTE_vii) {
			// Check keys for new note
			switch (StateManager.getState()) {
			case SequencerState::Stop:
				// play the note
				DacDriver.writeVout(active_dac, SeqDriver.getDacValue((uint8_t)TouchDriver.getChangedNote(),
						SeqDriver.getOctave()));
				if (TouchDriver.getAllPressedKeys()[TouchDriver.getChangedNote()]) {
					// Turn gate on
					// TODO: Why is this backwards of what I think it should be??
					// 	Either:
					//	(1) What I think is turning the gate on is turning it off - issue with bitwise manipulation
					// 		I think this is less likely!
					//  (2) I've got something backwards about pressing / releasing buttons
					//		But this also seems unlikely, since the key pressing seems to generally work
					//  So I guess there's a third thing??
//					*(pGatePortArray[active_sequencer]) |= gateArray[active_sequencer];
					*(pGatePortArray[active_sequencer]) &= ~gateArray[active_sequencer];
				}
				else {
					// Turn gate off
//					*(pGatePortArray[active_sequencer]) &= ~gateArray[active_sequencer];
					*(pGatePortArray[active_sequencer]) |= gateArray[active_sequencer];
				}
				break;
			case SequencerState::Record:
				// play the note
				Serial.print("Writing to index: "); Serial.println(SeqDriver.getThisIndex());
				DacDriver.writeVout(active_dac, SeqDriver.getDacValue((uint8_t)TouchDriver.getChangedNote(),
						SeqDriver.getOctave()));
				// record the note
				SeqDriver.setThisNote((uint8_t)TouchDriver.getChangedNote());
				SeqDriver.incrementIndex();
				if (SeqDriver.getThisIndex() == 0) {
					StateManager.setState(SequencerState::Stop);
				}
				break;
			default:
				// do nothing
				break;

			}

		}
		else if (allPressedKeys[BUTTON_OUT_1]) {
			// Change which output is currently being controlled
			active_sequencer = 0;
			active_color = sequencerColors[active_sequencer];
			active_dac = DacAddr::DACA;
		}
		else if (allPressedKeys[BUTTON_OUT_2]) {
			// Change which output is currently being controlled
			active_sequencer = 1;
			active_color = sequencerColors[active_sequencer];
			active_dac = DacAddr::DACB;
		}
		else if (allPressedKeys[BUTTON_OUT_3]) {
			// Change which output is currently being controlled
			active_sequencer = 2;
			active_color = sequencerColors[active_sequencer];
			active_dac = DacAddr::DACC;
		}
		else if (allPressedKeys[BUTTON_OUT_4]) {
			// Change which output is currently being controlled
			active_sequencer = 3;
			active_color = sequencerColors[active_sequencer];
			active_dac = DacAddr::DACD;
		}
		else if (allPressedKeys[BUTTON_OCT_UP]) {
			// Increase current octave
			SeqDriver.incrementOctave();
		}
		else if (allPressedKeys[BUTTON_OCT_DOWN]) {
			// Decrease current octave
			SeqDriver.decrementOctave();
		}
		else {
			// A button was released, do nothing
		}



		// Display pressed keys
		DisplayDriver.displayPressedKeys(allPressedKeys, active_color);
//		TouchDriver.printKeys(newPressedKeys);
		Serial.println(); Serial.flush();
		attachInterrupt(digitalPinToInterrupt(NCHANGE_PIN), nCHANGE_ISR, LOW);
	} // end if new touch

	if (StateManager.getState() == SequencerState::Play)
	{
		if (SeqDriver.updateOutput()) {
			DisplayDriver.step(SeqDriver.getId(), SeqDriver.getThisIndex());
		}
	}
	else if (StateManager.getState() == SequencerState::Record) {
		if (millis() > last_blink_time + recording_blink_time) {
			last_blink_time = millis();
			// TODO: toggle the play : pause button
			//	This function is called appropriately, but nothing happens.
			//  The toggling internal to this function also appears to work, alternating between
			//  the if and else clauses.
			//  I think maybe "displayPressedKeys" is getting in the way
			DisplayDriver.togglePlayPause(active_color);
		}
	}
}

