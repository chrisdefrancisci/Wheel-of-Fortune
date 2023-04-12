// Do not remove the include below
#include "Sequencer.h"

// Constants



// DAC
const uint8_t dac_driver_address = B0001100;

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
SequencerDriver	SeqDriver;
AD5695 DacDriver(dac_driver_address);
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

	pinMode(NCHANGE_PIN, INPUT); // TODO: !!! MUST ATTACH INTERRUPT FOR THIS TO WORK !!!
    attachInterrupt(digitalPinToInterrupt(NCHANGE_PIN), nCHANGE_ISR, LOW);

	Serial.println("Setup complete.");
	DisplayDriver.circleOff();


	SeqDriver.begin();

	// Turn on interrupts last
}

// The loop function is called in an endless loop
void loop()
{
	static uint8_t active_sequencer = 0;
	static rgb8_t active_color = sequencerColors[active_sequencer];
	Bitfield<QT1245_DETECT_BYTES> allPressedKeys;
	Bitfield<QT1245_DETECT_BYTES> newPressedKeys;

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
														  //	Although some debouncing may be necessary?s
		if (allPressedKeys[BUTTON_PLAY]) {
			// Check keys for state transition
			if (allPressedKeys[BUTTON_FUNC]) {
				// Start or stop recording
				if (StateManager.getState() == SequencerState::Stop){
					StateManager.setState(SequencerState::Record);
				}
				else {
					StateManager.setState(SequencerState::Stop);
				}
			}
			// If stopped, play. Otherwise, stop.
			if (StateManager.getState() == SequencerState::Stop){
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
				// TODO: replace DACA with selected DAC
				DacDriver.writeVout(DacAddr::DACA, SeqDriver.getDacValue((uint8_t)TouchDriver.getNewNote(),
						SeqDriver.getOctave()));
				break;
			case SequencerState::Record:
				// play the note
				// TODO: replace DACA with selected DAC
				DacDriver.writeVout(DacAddr::DACA, SeqDriver.getDacValue((uint8_t)TouchDriver.getNewNote(),
						SeqDriver.getOctave()));
				// record the note
				// SeqDriver...
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
		}
		else if (allPressedKeys[BUTTON_OUT_2]) {
			// Change which output is currently being controlled
			active_sequencer = 1;
			active_color = sequencerColors[active_sequencer];
		}
		else if (allPressedKeys[BUTTON_OUT_3]) {
			// Change which output is currently being controlled
			active_sequencer = 2;
			active_color = sequencerColors[active_sequencer];
		}
		else if (allPressedKeys[BUTTON_OUT_4]) {
			// Change which output is currently being controlled
			active_sequencer = 3;
			active_color = sequencerColors[active_sequencer];
		}
		else if (allPressedKeys[BUTTON_OCT_UP]) {
			// Increase current octave
		}
		else if (allPressedKeys[BUTTON_OCT_DOWN]) {
			// Decrease current octave
		}
		else {
			// A button was released, do nothing
		}



		// Display pressed keys
		DisplayDriver.displayPressedKeys(allPressedKeys, active_color);
//		TouchDriver.printKeys(newPressedKeys);
		Serial.println(); Serial.flush();
		attachInterrupt(digitalPinToInterrupt(NCHANGE_PIN), nCHANGE_ISR, LOW);
	}

	/** Sequencer interrupts */
	if (StateManager.getState() == SequencerState::Play)
	{
		if (SeqDriver.getStepFlag()){
			SeqDriver.clearStepFlag(); // Clear flag immediately
	//		Serial.print(sequence[SeqDriver.getThisIndex()], HEX); Serial.print(": ");
			// Write output to DAC
			//printWireStatus(DacDriver.writeVout(DACA, SeqDriver.getThisValue()));
			// Update display
			DisplayDriver.step(SeqDriver.getId(), SeqDriver.getThisIndex());
	//		*pDebugLedPort ^= debugLed; // Toggle LED
	//		LedDriver.writeLed(SeqDriver.getThisIndex(), BLACK_RGB);
	//		LedDriver.writeLed(SeqDriver.getNextIndex(), rgbConsts[rgbCircIdx]);


			// TODO: implement actual write functionality. Until then, this cycles through the five
			// octaves
			if (SeqDriver.getThisIndex() == SeqDriver.getSequenceLength() - 1) {
				static uint8_t octave = 0;
				for (uint8_t i = 0; i < SeqDriver.getSequenceLength(); i++) {
					SeqDriver.setValue(i, octave*SeqDriver.getSequenceLength() + i);
				}
				octave++;
				if (octave >= N_OCTAVES) {
					octave = 0;
				}
			}

			Serial.println("   Cleared stepFlag.");
		}
	}
}

