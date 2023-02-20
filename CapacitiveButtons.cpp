/*
 * CapacitiveButtons.cpp
 *
 *  Created on: Feb 7, 2023
 *      Author: Chris
 */

#include "CapacitiveButtons.h"

/**
 *
 * @param chip_select_pin
 * @param nCHANGE_pin
 * @param nDRDY_pin
 * @param this_chip_select_pin
 * @param DisplayDriver
 */
CapacitiveButtons::CapacitiveButtons(uint8_t chip_select_pin, uint8_t nCHANGE_pin, uint8_t nDRDY_pin, uint8_t this_chip_select_pin, Display* DisplayDriver):
	TouchDriver(chip_select_pin, nCHANGE_pin, nDRDY_pin, this_chip_select_pin),
	DisplayDriver(DisplayDriver)
{
	// TODO Auto-generated constructor stub

}

/**
 *
 */
CapacitiveButtons::~CapacitiveButtons() {
	// TODO Auto-generated destructor stub
}

/**
 *
 * @return
 */
CommStatus CapacitiveButtons::begin(void) {
	CommStatus status;
	status = TouchDriver.begin();
	return status;
}

/**
 *
 * @return
 */
Bitfield<QT1245_DETECT_BYTES> CapacitiveButtons::updatePressedKeys(void) {
	Bitfield<QT1245_DETECT_BYTES> oldPressedButtons = allPressedKeys;
	TouchDriver.getKeyStatus(allPressedKeys);
	newPressedKeys = oldPressedButtons ^ allPressedKeys;
	return newPressedKeys;
}

/**
 * Prints the keys using predefined strings in Hardware.h
 * @param printBuffer Bitfield of key presses to print.
 */
void CapacitiveButtons::printKeys(Bitfield<QT1245_DETECT_BYTES> printBuffer) {
	Serial.print("Pressed: "); Serial.flush();
	char buffer[15];
	for (uint8_t i = 0; i < QT1245_N_KEYS; i++) {
		if (printBuffer[i] == true) {
			getProgmemString(i, buffer);
			Serial.print(buffer); Serial.print(" ");
			Serial.print(i); Serial.print(" "); Serial.flush();
		}
	}
	Serial.println();
}

