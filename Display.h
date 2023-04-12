/*
 * Display.h
 *
 *  Created on: Nov 17, 2022
 *      Author: Chris
 *
 * This class instantiates the IS31FL3245_LED_driver specific to the application and
 * 		creates helper functions that allow for easy interfacing with the
 * 		SequencerDriver(s)
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "IS31FL3246_LED_driver.h"
#include "AT42_QT1245_Touch_driver.h" // TODO: should necessary constants from AT42..., others,
									  // 	be moved to remove so many interdependencies?
#include "SequencerDriver.h"
#include "Arduino.h"
#include "Bitfield.h"

// Colors
const rgb8_t OFF_RGB = {0, 0, 0};
const rgb8_t WHITE_RGB = {255, 255, 255};

const rgb8_t RED_RGB = {255, 0, 0};
const rgb8_t ORANGE_RGB = {255, 100, 0};
const rgb8_t YELLOW_RGB = {255, 255, 0};
const rgb8_t LIME_RGB = {100, 255, 0};

const rgb8_t GREEN_RGB = {0, 255, 0};
const rgb8_t SEA_RGB = {0, 255, 80};
const rgb8_t CYAN_RGB = {0, 255, 255};
const rgb8_t INDIGO_RGB = {0, 100, 255};

const rgb8_t BLUE_RGB = {0, 0, 255};
const rgb8_t PURPLE_RGB = {150, 0, 255};
const rgb8_t VIOLET_RGB = {255, 0, 255};
const rgb8_t MAGENTA_RGB = {255, 0, 50};
const rgb8_t rgbConsts[] = {RED_RGB, ORANGE_RGB, YELLOW_RGB, LIME_RGB, GREEN_RGB,
		SEA_RGB, CYAN_RGB, INDIGO_RGB, BLUE_RGB, PURPLE_RGB, VIOLET_RGB, MAGENTA_RGB};
const uint8_t rgbLen = 12;

const rgb8_t sequencerColors[] = { BLUE_RGB, MAGENTA_RGB, SEA_RGB, ORANGE_RGB };
const rgb8_t controlColor = VIOLET_RGB;
const rgb8_t errorColor = RED_RGB;

// LED Driver
const uint8_t circular_led_driver_address = B0110000; // AD connected to GND
const uint8_t peripheral_led_driver_address = B0110011; // AD connected to +5V
const uint8_t sdb_pin = 9;
const bool isRGB = true;
const bool is8bit = true;
const uint8_t ledLen = 12;

class Display {
public:
	Display();
	virtual ~Display();

	void begin();
	bool rainbowLoop(unsigned long display_millis);
	void circleOff();
	void peripheralOff();
	void step(uint8_t sequencer_id, uint8_t this_index);
	void displayPressedKeys(Bitfield<QT1245_DETECT_BYTES> pressedKeys, rgb8_t color = WHITE_RGB);

private:
	IS31FL3246_LED_driver circular_led_driver; // Driver for LEDs in a circle, with address b0110 000x
	IS31FL3246_LED_driver peripheral_led_driver; // Driver for LEDs in the corner, with address b0110 011x
	uint8_t last_LED[N_SEQUENCERS]; // Keep track of which LEDs need to be cleared on the next sequencer step

};



#endif /* DISPLAY_H_ */
