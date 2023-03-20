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
	CommStatus status = CommStatus::Success;
	uint8_t new_BL = 0; // Change burst length from default to reduce sensitivity
	uint8_t new_AKS = 1; // Enable adjacent key suppression
	uint8_t new_THRM = 2; // Change threshold multiplier from default x1 --> x4 to reduce sensitivity
	uint8_t ADDR_244; // Address containing DWELL, RIB, THRM, FHM
	keySetups_t verify_setups[QT1245_N_KEYS];
	// attempt to initialize device
	while(TouchDriver.begin() != CommStatus::Success)
	{
		Serial.println("Unable to contact sensor. Waiting 10 seconds.");
		Serial.flush();
		delay(10000);
	}
	// Write to EEPROM registers below - only need to be called once per device
//	status |= TouchDriver.setKeySetupsBL(new_BL, 0, QT1245_N_KEYS);
//	status |= TouchDriver.setKeySetupsAKS(new_AKS, 0, QT1245_N_KEYS);
	// Threshold Multiplier THRM
	TouchDriver.readData(QT1245_DWELL_RIB_THRM_FHM_ADDR, 1, &ADDR_244);
	Serial.print("Initial ADDR 244 = "); Serial.print(ADDR_244, BIN);
	// Clear or set bits for THRM
	// TODO: find a better way to do this by setting two bits at once using bitfield class
	// and casting bitfield to uint8_t
//	ADDR_244 = new_THRM & 1 ? ADDR_244 & ~(new_THRM << QT1245_THRM_BIT) :
//			ADDR_244 | (new_THRM << QT1245_THRM_BIT);
//	ADDR_244 = new_THRM & (1 << 1) ? ADDR_244 & ~(new_THRM << (QT1245_THRM_BIT + 1)) :
//			ADDR_244 | (new_THRM << (QT1245_THRM_BIT + 1));
//	TouchDriver.writeData(QT1245_DWELL_RIB_THRM_FHM_ADDR, ADDR_244);

	TouchDriver.restart(); // Restart is required after writing to setups

	TouchDriver.readData(QT1245_DWELL_RIB_THRM_FHM_ADDR, 1, &ADDR_244);
	Serial.print(" -> Updated ADDR 244 = "); Serial.println(ADDR_244, BIN);

	status |= TouchDriver.getKeySetups(verify_setups, 0, QT1245_N_KEYS-1);
	for (uint8_t i = 0; i < QT1245_N_KEYS; i ++ ) {
		Serial.println(); Serial.print("Key "); Serial.println(i);

		Serial.print("\t\tAddress "); Serial.print(QT1245_SETUPS0_ADDR + i); Serial.print(": ");
		Serial.print(verify_setups[i].uint8_t0.data, BIN);
		Serial.print("\t BL "); Serial.print(verify_setups[i].uint8_t0.bitfield.BL);
		Serial.print("\t NDRIFT "); Serial.print(verify_setups[i].uint8_t0.bitfield.NDRIFT);
		Serial.print("\t NTHR "); Serial.println(verify_setups[i].uint8_t0.bitfield.NTHR);
		Serial.print("\t\tAddress "); Serial.print(QT1245_SETUPS0_ADDR + i); Serial.print(": ");
		Serial.print(verify_setups[i].uint8_t1.data, BIN);
		Serial.print("\t WAKE "); Serial.print(verify_setups[i].uint8_t1.bitfield.WAKE);
		Serial.print("\t AKS "); Serial.print(verify_setups[i].uint8_t1.bitfield.AKS);
		Serial.print("\t FDIL "); Serial.print(verify_setups[i].uint8_t1.bitfield.FDIL);
		Serial.print("\t NDIL "); Serial.println(verify_setups[i].uint8_t1.bitfield.NDIL);
	}
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
			Serial.print(buffer); Serial.print(" "); Serial.flush();
		}
	}
	Serial.println();
}

