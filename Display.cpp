/*
 * Display.cpp
 *
 *  Created on: Nov 17, 2022
 *      Author: Chris
 */

#include "Display.h"


const uint8_t index2led[] = {0, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};

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

void Display::begin() {
	circular_led_driver.begin();
	peripheral_led_driver.begin();
}

/**
 *
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


void Display::circleOff() {
	for (uint8_t ledIdx = 0; ledIdx < ledLen; ledIdx++){
		circular_led_driver.writeLed(ledIdx, OFF_RGB);
	}
	circular_led_driver.update();
}


void Display::peripheralOff() {
	for (uint8_t ledIdx = 0; ledIdx < ledLen; ledIdx++){
		peripheral_led_driver.writeLed(ledIdx, OFF_RGB);
	}
}


void Display::step(uint8_t sequencer_id, uint8_t this_index) {
	// Turn off previous LED, save current LED as previous
	circular_led_driver.writeLed(last_LED[sequencer_id], OFF_RGB);
	last_LED[sequencer_id] = index2led[this_index];
	circular_led_driver.writeLed(index2led[this_index], sequncerColors[sequencer_id]);
	circular_led_driver.update();
}
