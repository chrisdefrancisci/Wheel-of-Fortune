/*
 * CapacitiveButtons.h
 *
 *  Created on: Feb 7, 2023
 *      Author: Chris
 */

#ifndef CAPACITIVE_BUTTONS_H_
#define CAPACITIVE_BUTTONS_H_

#include "Arduino.h"
#include "AT42_QT1245_Touch_driver.h"
#include "Display.h"
#include "Hardware.h"
#include "Bitfield.h"
#include "CommStatus.h"

class CapacitiveButtons {
public:
	CapacitiveButtons(uint8_t chip_select_pin, uint8_t nCHANGE_pin, uint8_t nDRDY_pin, uint8_t this_chip_select_pin, Display* DisplayDriver);
	virtual ~CapacitiveButtons(void);
	CommStatus begin(void);
	Bitfield<QT1245_DETECT_BYTES> updatePressedKeys(void);
	void printKeys(Bitfield<QT1245_DETECT_BYTES> printBuffer);
//	bool getKey(uint8_t key);
	uint8_t getChangedNote(void);
	inline Bitfield<QT1245_DETECT_BYTES> getAllPressedKeys(void) { return allPressedKeys; }
	inline Bitfield<QT1245_DETECT_BYTES> getNewChangedKeys(void) { return newChangedKeys; }
	inline Bitfield<QT1245_DETECT_BYTES> getNewPressedKeys(void) { return newPressedKeys; }
private:
	AT42_QT1245_Touch_driver TouchDriver;
	Display* DisplayDriver;
	Bitfield<QT1245_DETECT_BYTES> allPressedKeys;
	Bitfield<QT1245_DETECT_BYTES> newChangedKeys;
	Bitfield<QT1245_DETECT_BYTES> newPressedKeys;
};

#endif /* CAPACITIVE_BUTTONS_H_ */
