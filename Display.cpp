/*
 * Display.cpp
 *
 *  Created on: Nov 17, 2022
 *      Author: Chris
 */

#include "Display.h"


const uint8_t button2led[N_RGB_LEDS*2] = {0, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, // Ring indices
		2, 4, 3, 5, 6, 0, 7, 1}; // Peripheral indices

Display::Display() :
	circular_led_driver(circular_led_driver_address, sdb_pin, isRGB, is8bit),
	peripheral_led_driver(peripheral_led_driver_address, sdb_pin, isRGB, is8bit)
{

}

/**
 * Destructor
 */
Display::~Display() {
	// TODO Auto-generated destructor stub
}

/**
 * Sets up instance members.
 */
void Display::begin() {
	circular_led_driver.begin();
	peripheral_led_driver.begin();
}

/**
 * Creates a cute little rainbow animation. Non-blocking.
 * @param display_millis The time for each step to display
 * @return True if loop cycle has completed, otherwise false.
 */
bool Display::rainbowLoop(unsigned long display_millis){
	static unsigned long last_millis = 0;
	unsigned long this_millis = millis();
	static int rgbIdx = 0;
	if (this_millis - last_millis >= display_millis) {
		last_millis = millis();

		for (int ledIdx = 0; ledIdx < ledLen; ledIdx++) {
			int rgbCircIdx = ((rgbIdx + ledIdx) >= rgbLen ) ? rgbIdx + ledIdx - rgbLen : rgbIdx + ledIdx;
			circular_led_driver.writeLed(ledIdx, rgbConsts[rgbCircIdx]);
		}
		circular_led_driver.update();
		rgbIdx++;
		if (rgbIdx >= rgbLen) {
			rgbIdx = 0;
			return true;
		}
	}
	return false;
}

/**
 * Turns the entire set of circular LEDs.
 */
void Display::circleOff() {
	for (uint8_t ledIdx = 0; ledIdx < ledLen; ledIdx++){
		circular_led_driver.writeLed(ledIdx, OFF_RGB);
	}
	circular_led_driver.update();
}

/**
 * Turns of the entire set of peripheral LEDs.
 */
void Display::peripheralOff() {
	for (uint8_t ledIdx = 0; ledIdx < ledLen; ledIdx++){
		peripheral_led_driver.writeLed(ledIdx, OFF_RGB);
	}
}

/**
 * Illuminates the next LED in the sequence.
 * @param[in] sequencer_id The sequencer to grab the last index from.
 * @param[in] this_index The current index to write.
 */
void Display::step(uint8_t sequencer_id, uint8_t this_index) {
	// Turn off previous LED, save current LED as previous
	circular_led_driver.writeLed(last_LED[sequencer_id], OFF_RGB);
	last_LED[sequencer_id] = button2led[this_index];
	circular_led_driver.writeLed(button2led[this_index], sequencerColors[sequencer_id]);
	circular_led_driver.update();
}

/**
 * Uses the RGB LEDs to light up the panel, displaying the keys that are currently pressed,
 * turns off all others.
 * @param[in] pressedKeys Bitfield containing 1 where keys are pressed, 0 where they are not
 * @param[in] color The color for the pressed keys
 */
void Display::displayPressedKeys(Bitfield<QT1245_DETECT_BYTES> pressedKeys, rgb8_t color) {
	// Turn on LEDs in first driver if touched, otherwise turn them off
	for (uint8_t i = 0; i < QT1245_DETECT_BYTES * 8; i++) {
		Serial.print((uint8_t)pressedKeys[i]);
		if (i%12 ==0){
			Serial.print(" ");
		}
	}
	for (uint8_t i = 0; i < N_RGB_LEDS; i++) {
		if (pressedKeys[i]) {
			circular_led_driver.writeLed(button2led[i], color);
			Serial.print("\tDisplay "); Serial.print(i); Serial.print(" from "); Serial.print(button2led[i]);
		}
		else {
			circular_led_driver.writeLed(button2led[i], OFF_RGB);
		}
	}
	Serial.println();
	circular_led_driver.update();
	// Turn on LEDs in second driver
	for (uint8_t i = 0; i < N_RGB_LEDS; i++) {
		uint8_t complete_idx = i + N_RGB_LEDS;
		if (pressedKeys[complete_idx]) {
			peripheral_led_driver.writeLed(button2led[complete_idx], color);
			Serial.print("\tDisplay "); Serial.print(i); Serial.print(" from "); Serial.print(button2led[i]);
		}
		else {
			peripheral_led_driver.writeLed(button2led[complete_idx], OFF_RGB);
		}
	}
	Serial.println();
	peripheral_led_driver.update();
}
