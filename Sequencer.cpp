// Do not remove the include below
#include "Sequencer.h"

using namespace IS31FL3246;

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
uint16_t* sequence = new uint16_t[SeqDriver.getMaxLength()];

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
	SeqDriver.begin(); // Turn on interrupts last

	while (!DisplayDriver.rainbowLoop(100)){

	}
//	while (LedDriver.begin() != WireStatus::SUCCESS) {
//		delay(5);
//	}
	// Write initial values to be updated on first tick of timer
//	LedDriver.writeLed(SeqDriver.getNextIndex(), rgbConsts[0]);

	/* TODO: Touch sensor code
	// start the SPI library
	SPI.begin();
	pinMode(chip_select_pin, OUTPUT);
	pinMode(arduino_select_pin, OUTPUT);
	//  pinMode(11, OUTPUT); These shouldn't be necessary
	//  pinMode(12, INPUT);

	// nCHANGE pin interrupt
	pinMode(nCHANGE_pin, INPUT);
	attachInterrupt(digitalPinToInterrupt(nCHANGE_pin), nCHANGE_ISR, LOW);
	pinMode(nDRDY_pin, INPUT);
	// attempt to initialize device
	while(!AT42QT1245_init())
	{
		Serial.println("Unable to contact sensor. Waiting 2 seconds.");
		delay(2000);
	}

	// reduce sensitivity for all Y0 keys
	int keyStart = 0;
	int nKeys = 8;
	//  reduceSensitivity(keyStart, nKeys);
	keySetups_t* pSetups = new keySetups_t[nKeys];
	getKeySetups(pSetups, keyStart, nKeys);
	for(int i = 0; i < nKeys; i++) {
		Serial.print("For key "); Serial.print(i);
		Serial.print(", BL = "); Serial.print((pSetups+i)->byte0.BL);
		Serial.print("  NDRIFT = "); Serial.print((pSetups+i)->byte0.NDRIFT);
		Serial.print("  NTHR = "); Serial.print((pSetups+i)->byte0.NTHR);
		Serial.print("  | WAKE = "); Serial.print((pSetups+i)->byte1.WAKE);
		Serial.print("  AKS = "); Serial.print((pSetups+i)->byte1.AKS);
		Serial.print("  FDIL = "); Serial.print((pSetups+i)->byte1.FDIL);
		Serial.print("  NDIL = "); Serial.println((pSetups+i)->byte1.NDIL);
	}

	for (int i = 0; i < nKeys; i++)
	{
	(pSetups+i)->byte0.BL = 1;
	(pSetups+i)->byte0.NDRIFT = 3;
	(pSetups+i)->byte0.NTHR = 3;
	(pSetups+i)->byte1.WAKE = 1;
	(pSetups+i)->byte1.AKS = 0; // helps for the button, but definitely messes up the sliders
	(pSetups+i)->byte1.FDIL = 3;
	(pSetups+i)->byte1.NDIL = 2;
	}
	setKeySetups(pSetups, keyStart, nKeys);
	setKeySetups(pSetups, 23, 0); // button Y2X7

	getKeySetups(pSetups, keyStart, nKeys);
	for(int i = 0; i < nKeys; i++) {
		Serial.print("For key "); Serial.print(i);
		Serial.print(", BL = "); Serial.print((pSetups+i)->byte0.BL);
		Serial.print("  NDRIFT = "); Serial.print((pSetups+i)->byte0.NDRIFT);
		Serial.print("  NTHR = "); Serial.print((pSetups+i)->byte0.NTHR);
		Serial.print("  | WAKE = "); Serial.print((pSetups+i)->byte1.WAKE);
		Serial.print("  AKS = "); Serial.print((pSetups+i)->byte1.AKS);
		Serial.print("  FDIL = "); Serial.print((pSetups+i)->byte1.FDIL);
		Serial.print("  NDIL = "); Serial.println((pSetups+i)->byte1.NDIL);
	}

	KeyStatus_t keyStatus = {0};
	pKeyStatus_t pKeyStatusUnion;
	pKeyStatusUnion.pKeyStatus = &keyStatus;
	getKeyStatus(pKeyStatusUnion);
	*/ // end touch sensor code


	Serial.println("Setup complete.");
	for (uint8_t i = 0; i < SeqDriver.getMaxLength(); i++){
		sequence[i] = rand() % 16384;
		Serial.print("Initialized sequence["); Serial.print(i);
		Serial.print("] to "); Serial.println(sequence[i]);
	}
	DisplayDriver.circleOff();
}

// The loop function is called in an endless loop
void loop()
{
//Add your repeated code here

	/* TODO Touch sensor code
	//  const int poll_time = 2000; // ms
	//  static unsigned long int last_poll = 0;
	//  unsigned long int this_poll = millis();
	  // store current data
	  KeyStatus_t keyStatus = {0};
	  pKeyStatus_t pKeyStatusUnion;
	  pKeyStatusUnion.pKeyStatus = &keyStatus;

	  // store last data
	//  KeyStatus_t lastKeyStatus = {0};
	//  pKeyStatus_t pLastKeyStatusUnion;
	//  pLastKeyStatusUnion.pKeyStatus = &lastKeyStatus;

	  // transfer data from device
	  if(new_touch)
	  {
	    new_touch = false;
	    Serial.print("New touch detected: "); Serial.flush();

	    getKeyStatus(pKeyStatusUnion);
	//    printKeyStatusChange(pLastKeyStatusUnion, pKeyStatusUnion);
	    printPressedKeys(pKeyStatusUnion.pKeyStatusByte);
	    Serial.println(); Serial.flush();
	    attachInterrupt(digitalPinToInterrupt(nCHANGE_pin), nCHANGE_ISR, LOW);
	  }
	//  if (this_poll - last_poll > poll_time)
	//  {
	//    Serial.print("Polling...");
	//    getKeyStatus(pKeyStatusUnion);
	//    printKeyStatusChange(pLastKeyStatusUnion, pKeyStatusUnion);
	//    Serial.println();
	//    last_poll = this_poll;
	//  }
	 */
	/** Sequencer interrupts */
	static int rgbCircIdx = 0;
	if (SeqDriver.getStepFlag()){
		SeqDriver.clearStepFlag(); // Clear flag immediately
//		Serial.print(sequence[SeqDriver.getThisIndex()], HEX); Serial.print(": ");
		// Write output to DAC
		WireStatus::printWireStatus(DacDriver.writeVout(DAC_ALL, sequence[SeqDriver.getThisIndex()]));
		// Update display
		DisplayDriver.step(SeqDriver.getId(), SeqDriver.getThisIndex());
//		*pDebugLedPort ^= debugLed; // Toggle LED
//		LedDriver.writeLed(SeqDriver.getThisIndex(), BLACK_RGB);
//		LedDriver.writeLed(SeqDriver.getNextIndex(), rgbConsts[rgbCircIdx]);
		Serial.println(" Exiting.");
	}

}

