// Do not remove the include below
#include "Sequencer.h"

using namespace IS31FL3246;

// Constants

// LED Driver
const uint8_t led_driver_address = B0110000; // AD connected to GND
//const uint8_t led_driver_address = B0110011; // AD connected to +5V
const uint8_t sdb_pin = 9;
const bool isRGB = true;
const bool is8bit = true;
const uint8_t ledLen = 12;

// Colors
const rgb8_t RED_RGB = {255, 0, 0};
const rgb8_t ORANGE_RGB = {255, 100, 0};
const rgb8_t YELLOW_RGB = {255, 255, 0};
const rgb8_t LIME_RGB = {100, 255, 0};

const rgb8_t GREEN_RGB = {0, 255, 0};
const rgb8_t SEA_RGB = {0, 255, 80};
const rgb8_t CYAN_RGB = {0, 255, 255};
const rgb8_t INDIGO_RGB = {0, 200, 255};

const rgb8_t BLUE_RGB = {0, 0, 255};
const rgb8_t PURPLE_RGB = {150, 0, 255};
const rgb8_t VIOLET_RGB = {255, 0, 255};
const rgb8_t MAGENTA_RGB = {255, 0, 50};
rgb8_t rgbConsts[] = {RED_RGB, ORANGE_RGB, YELLOW_RGB, LIME_RGB, GREEN_RGB,
		SEA_RGB, CYAN_RGB, INDIGO_RGB, BLUE_RGB, PURPLE_RGB, VIOLET_RGB, MAGENTA_RGB};
const uint8_t rgbLen = 12;

// Global variables
IS31FL3246_LED_driver LedDriver(led_driver_address, sdb_pin, isRGB, is8bit);

//The setup function is called once at startup of the sketch
void setup()
{
	// put your setup code here, to run once:
	Serial.begin(9600);
	while (LedDriver.begin() != WireStatus::SUCCESS) {

	}

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
	*/
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


// LED driver loop

	//TODO: create animation functions, let this be rainbow loop
	static unsigned long last_millis = 0;
	unsigned long this_millis = millis();
	unsigned long interval_millis = 200;
	static int rgbIdx = 0;
	if (this_millis - last_millis >= interval_millis) {
		last_millis = millis();

		for (int ledIdx = 0; ledIdx < ledLen; ledIdx++) {
			int rgbCircIdx = ((rgbIdx + ledIdx) >= rgbLen ) ? rgbIdx + ledIdx - rgbLen : rgbIdx + ledIdx;
			LedDriver.writeLed(ledIdx, rgbConsts[rgbCircIdx]);
		}
		LedDriver.update();
		rgbIdx++;
		if (rgbIdx >= rgbLen) {
			rgbIdx = 0;
		}
	}
}

