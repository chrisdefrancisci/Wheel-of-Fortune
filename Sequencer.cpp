// Do not remove the include below
#include "Sequencer.h"

// DAC
//const uint8_t dac_driver_address = B0001100;

// Other
// TODO find a better way to do pin assignments that doesn't involve converting to and from
// 		the arduino pin convention


// Global variables
Display DisplayDriver;
AD5695 DacDriver(DAC_DRIVER_ADDRESS);
SequencerDriver	SeqDriver(&DacDriver); // TODO: This will only create the first sequencer, I need to create 3 more
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
//	*pGate0Setup |= gate0; // Set to output (1)
//	*pGate0Port &= ~gate0; // Set low
//	*pGate1Setup |= gate1; // Set to output (1)
//	*pGate1Port &= ~gate1; // Set low
//	*pGate2Setup |= gate2; // Set to output (1)
//	*pGate2Port &= ~gate2; // Set low
//	*pGate3Setup |= gate3; // Set to output (1)
//	*pGate3Port &= ~gate3; // Set low

	// Communication setups

	Serial.begin(9600);
	Serial.println("Initializing...");

	DacDriver.begin();

	// Peripheral setups
	DisplayDriver.begin();

	// Initialize device, set EEPROM if needed
	TouchDriver.begin();

	// Set interrupt for pin that signals change in touch
	pinMode(NCHANGE_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(NCHANGE_PIN), nCHANGE_ISR, LOW);

	Serial.println("Setup complete.");
	DisplayDriver.displayState(SequencerState::Stop);
	DisplayDriver.update();


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
	static uint8_t gate_length_fraction = 11; // Default to notes that are full length but not slur


	// Current user input
	Bitfield<QT1245_DETECT_BYTES> allPressedKeys;
	Bitfield<QT1245_DETECT_BYTES> newPressedKeys;
	// Visual indicators
	static uint32_t last_time = 0;
	if (millis() > last_time + 5000) { // TODO: delete this
		uint8_t bpm = SeqDriver.getBPM();
		if (bpm > 120) {
			bpm = 60;
		}
		else {
			bpm *= 2;
		}
		SeqDriver.setBPM(bpm);

		last_time = millis();
	}


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
				if (StateManager.getState() == SequencerState::Stop){
					StateManager.setState(SequencerState::Record);
					Serial.print("Record state ");
					SeqDriver.restartIndex();
				}
				else {
					StateManager.setState(SequencerState::Stop);
					Serial.print("Stop state ");
				}
			}
			// If stopped, play. Otherwise, stop.
			else if (StateManager.getState() == SequencerState::Stop){
				SeqDriver.restartIndex();
				StateManager.setState(SequencerState::Play);
				Serial.print("Play state");
			}
			else {
				StateManager.setState(SequencerState::Stop);
				SeqDriver.gateOff(); // TODO: All gates
				Serial.print("Stop state");
			}
			DisplayDriver.displayState(StateManager.getState());
		}
		else if (TouchDriver.getChangedNote() >= NOTE_i && TouchDriver.getChangedNote() <= NOTE_vii) {
			// Check keys for new note
			switch (StateManager.getState()) {
			case SequencerState::Stop:
				// play the note
				DacDriver.writeVout(active_dac, SeqDriver.getDacValue((uint8_t)TouchDriver.getChangedNote(),
						SeqDriver.getOctave()));
				if (TouchDriver.getAllPressedKeys()[TouchDriver.getChangedNote()]) {
					// TODO: replace with array of drivers
					SeqDriver.gateOn();
				}
				else {
					// TODO: replace with array of drivers
					SeqDriver.gateOff();
				}
				break;
			case SequencerState::Record:
				// play the note
				Serial.print("Writing to index: "); Serial.println(SeqDriver.getThisIndex());
				DacDriver.writeVout(active_dac, SeqDriver.getDacValue((uint8_t)TouchDriver.getChangedNote(),
						SeqDriver.getOctave()));
				if (TouchDriver.getAllPressedKeys()[TouchDriver.getChangedNote()]) {
					// TODO: replace with array of drivers
					SeqDriver.gateOn();
					// record the note
					SeqDriver.setThisPitch((uint8_t)TouchDriver.getChangedNote()); // TODO: replace with array of drivers
					// TODO: replace with array of drivers, replace with current articulation settings
//					SeqDriver.setThisGateLength(gate_length_fraction); // TODO: might need a more intuitive way of describing this, since numbers from 0-12 aren't really a fraction
					SeqDriver.incrementIndex();
					if (SeqDriver.getThisIndex() == 0) {
						StateManager.setState(SequencerState::Stop);
					}
				}
				else {
					// TODO: replace with array of drivers
					SeqDriver.gateOff();
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
			// DisplayDriver.setActiveColor(sequencerColors[active_sequencer]); // TODO
			DisplayDriver.displaySequencer(active_sequencer);
			active_dac = DacAddr::DACA;
		}
		else if (allPressedKeys[BUTTON_OUT_2]) {
			// Change which output is currently being controlled
			active_sequencer = 1;
			active_color = sequencerColors[active_sequencer];
			// DisplayDriver.setActiveColor(sequencerColors[active_sequencer]); // TODO
			DisplayDriver.displaySequencer(active_sequencer);
			active_dac = DacAddr::DACB;
		}
		else if (allPressedKeys[BUTTON_OUT_3]) {
			// Change which output is currently being controlled
			active_sequencer = 2;
			active_color = sequencerColors[active_sequencer];
			// DisplayDriver.setActiveColor(sequencerColors[active_sequencer]); // TODO
			DisplayDriver.displaySequencer(active_sequencer);
			active_dac = DacAddr::DACC;
		}
		else if (allPressedKeys[BUTTON_OUT_4]) {
			// Change which output is currently being controlled
			active_sequencer = 3;
			active_color = sequencerColors[active_sequencer];
			// DisplayDriver.setActiveColor(sequencerColors[active_sequencer]); // TODO
			DisplayDriver.displaySequencer(active_sequencer);
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
//		DisplayDriver.displayPressedKeys(allPressedKeys, active_color); // TODO deprecated
		DisplayDriver.displayAllPressedKeys(allPressedKeys);
//		TouchDriver.printKeys(newPressedKeys);
		Serial.println(); Serial.flush();
		attachInterrupt(digitalPinToInterrupt(NCHANGE_PIN), nCHANGE_ISR, LOW);
	} // end if new touch

	if (StateManager.getState() == SequencerState::Play)
	{
		SequencerData seq_data;
		if (SeqDriver.updateOutput(seq_data)) { // TODO: Need to have an array of 4 Sequencer drivers, iterate over them
//			DisplayDriver.step(SeqDriver.getId(), SeqDriver.getThisIndex()); // deprecated
			DisplayDriver.displayStep(SeqDriver.getId(), seq_data.step);
			DisplayDriver.displayGate(SeqDriver.getId(), seq_data.gate_on);
//			DisplayDriver.step(active_sequencer, SeqDriver.getThisIndex());
		}
	}
	else if (StateManager.getState() == SequencerState::Record) {

	}
	DisplayDriver.update();

}

