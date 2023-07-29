// Do not remove the include below
#include "Sequencer.h"

// Constants
const uint16_t recording_blink_time = 500; // Visual indicator for recording


// DAC
//const uint8_t dac_driver_address = B0001100;

// Other
// TODO find a better way to do pin assignments that doesn't involve converting to and from
// 		the arduino pin convention
volatile uint8_t* pDebugLedSetup = &DDRC;
volatile uint8_t* pDebugLedPort = &PORTC;
const uint8_t debugLed = 1 << 0;

volatile uint8_t* pDevResetSetup = &DDRB;
volatile uint8_t* pDevResetPort = &PORTB;
const uint8_t devReset = 1 << 1;

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
	*pDebugLedSetup |= debugLed; // Set to output (1)
//	*pDebugLedPort |= debugLed; // Set high
	*pDevResetSetup |= devReset; // Set to output (1)
	*pDevResetPort |= devReset; // Set high

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
	static uint16_t last_blink_time = 0;

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


		if (allPressedKeys[BUTTON_PLAY]) {
			// Check keys for state transition
			if (allPressedKeys[BUTTON_FUNC]) {
				// Start or stop recording
				if (StateManager.getState() == SequencerState::Stop){
					StateManager.setState(SequencerState::Record);
					SeqDriver.restartIndex();
				}
				else {
					StateManager.setState(SequencerState::Stop);
				}
			}
			// If stopped, play. Otherwise, stop.
			if (StateManager.getState() == SequencerState::Stop){
				SeqDriver.restartIndex();
				StateManager.setState(SequencerState::Play);
			}
			else {
				StateManager.setState(SequencerState::Stop);
			}
		}
		else if (TouchDriver.getNewNote() >= NOTE_i && TouchDriver.getNewNote() <= NOTE_vii) {
			// Check keys for new note
			switch (StateManager.getState()) {
			case SequencerState::Stop:
				// play the note
				DacDriver.writeVout(active_dac, SeqDriver.getDacValue((uint8_t)TouchDriver.getNewNote(),
						SeqDriver.getOctave()));
				break;
			case SequencerState::Record:
				// play the note
				DacDriver.writeVout(active_dac, SeqDriver.getDacValue((uint8_t)TouchDriver.getNewNote(),
						SeqDriver.getOctave()));
				// record the note
				SeqDriver.setThisNote((uint8_t)TouchDriver.getNewNote());
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
			// TODO: toggle the play : pause button
			DisplayDriver.togglePlayPause(active_color);
		}
	}
}

