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

	// attempt to initialize device
	while(TouchDriver.begin() != CommStatus::Success)
	{
		Serial.println("Unable to contact sensor. Waiting 2 seconds.");
		delay(2000);
	}
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
	//Add your repeated code here
	Bitfield<QT1245_DETECT_BYTES> newPressedKeys;

	Serial.flush();
	  // transfer data from device
	if(new_touch)
	{
		// TODO troubleshoot this, looks like printKeys causes a hangup (don't comment out updatePressedKeys!)
		// it looks like way too many things get printed
		new_touch = false;
		Serial.print("New touch detected: "); Serial.flush();
		TouchDriver.updatePressedKeys(); // TODO this line is the issue! Gets stuck somewhere
		// Potentially issue with my fancy bitfield type
		newPressedKeys = TouchDriver.getNewPressedKeys();
		TouchDriver.printKeys(newPressedKeys);
		Serial.println(); Serial.flush();
		attachInterrupt(digitalPinToInterrupt(NCHANGE_PIN), nCHANGE_ISR, LOW);
	}

	/** Sequencer interrupts */
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

