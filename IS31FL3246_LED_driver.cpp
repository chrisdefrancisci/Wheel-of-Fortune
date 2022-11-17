/*
 * IS31FL3246_LED_driver.cpp
 *
 *  Created on: Oct 18, 2022
 *      Author: Chris
 */

#include "IS31FL3246_LED_driver.h"

using namespace IS31FL3246;

/**
 * Class for the 12 groups of RGB LEDs
 * TODO: do I need this? Why?
 * Might be better off in a wrapper library
 * @param val
 */
//rgbGroup::rgbGroup(int val) {
//	if(val > max) {
//		_val  = max;
//	}
//	else if (val < min) {
//		_val = min;
//	}
//	else{
//		_val = val;
//	}
//}

/**
 *
 * @return
 */
//uint8_t rgbGroup::value(){
//	return _val;
//}

/**
 * Constructor
 * @param isRGB Denotes whether the driver controls 13 groups of 3(RGB) LEDs or 39 unassociated LEDs
 */
IS31FL3246_LED_driver::IS31FL3246_LED_driver(uint8_t led_driver_address, uint8_t sdb_pin, bool isRGB=true, bool is8bit=true) :
	_led_driver_address(led_driver_address),
	_sdb_pin(sdb_pin),
	_isRGB(isRGB),
	_is8bit(is8bit)
{
	// TODO Auto-generated constructor stub

}

/**
 * Destructor
 */
IS31FL3246_LED_driver::~IS31FL3246_LED_driver() {
	// TODO Auto-generated destructor stub
}

/**
 * Initializes communication with the LED driver.
 */
WireStatus::WireStatus IS31FL3246_LED_driver::begin() {
	WireStatus::WireStatus status = WireStatus::SUCCESS;
	Wire.begin();
	Serial.println("BEGIN");
	// setup IO pins
	pinMode(_sdb_pin, OUTPUT);
	// Set SDB pin high - enable LED driver
	digitalWrite(_sdb_pin, HIGH);
	Wire.beginTransmission(_led_driver_address); // slave address
	Wire.write(RESET_REG); // register address
	Wire.write(0xFF); // data
	status = printWireError(Wire.endTransmission());
	if(status != WireStatus::SUCCESS) {
		Serial.println("write(RESET_REG) 0xFF failed");
		return status;
	}
	// Set control register: 8-bit mode, normal operation
	Wire.beginTransmission(_led_driver_address); // slave address
	Wire.write(CONTROL_REG); // register address
	Wire.write(uint8_t(~(1<<PMS_BIT) & (1<<SSD_BIT) )); // data: 8-bit mode, turn on normal operation
	status = printWireError(Wire.endTransmission());
	if(status != WireStatus::SUCCESS) {
		Serial.println("write(CONTROL_REG) PMS_BIT SSD_BIT failed");
		return status;
	}
	// Set global current
//	Wire.beginTransmission(LED_DRIVER_ADDRESS); // slave address
//	Wire.write(GLOBAL_CURRENT_CTRL_REG_G); // register address
//	Wire.write(GCCG); // data
//	Wire.write(GCCR); // data, address auto increments
//	Wire.write(GCCB); // data, address auto increments
//	printWireError(Wire.endTransmission());
	rgb8_t globalCurrent = {GCCR, GCCG, GCCB};
	setRgbCurrent(globalCurrent);

	// Turn on LFP registers
	// TODO: probably need to do this for all LEDs; what is this constant 0x80?
	Wire.beginTransmission(_led_driver_address); // slave address
//	Wire.write(LFP_DUTY_REG + RGB_GROUP*RGB_GROUP_SIZE); // register address
	Wire.write(LFP_DUTY_REG);
	for (int i = 0; i < N_LEDS; i++){
		Wire.write(0x80);
		// TODO: determine if LFP register introduces noise, if so,
		// change to 0xFF
	}
	status = printWireError(Wire.endTransmission());
	if(status != WireStatus::SUCCESS) {
		Serial.println("write(CONTROL_REG) PMS_BIT SSD_BIT failed");
		return status;
	}
	Serial.println("END BEGIN");

	return status;
}


/**
 *
 * @param current
 */
void IS31FL3246_LED_driver::setRCurrent(uint8_t current) {
	// TODO: setRCurrent
}

/**
 *
 * @param current
 */
void IS31FL3246_LED_driver::setGCurrent(uint8_t current) {
	// TODO: setGCurrent
}

/**
 *
 * @param current
 */
void IS31FL3246_LED_driver::setBCurrent(uint8_t current) {
	// TODO: setBCurrent
}

/**
 *
 * @param currentRgb
 */
void IS31FL3246_LED_driver::setRgbCurrent(rgb8_t currentRgb) {
	Wire.beginTransmission(_led_driver_address); // slave address
	Wire.write(GLOBAL_CURRENT_CTRL_REG_G); // register address
	Wire.write(currentRgb.g); // data
	Wire.write(currentRgb.r); // data, address auto increments
	Wire.write(currentRgb.b); // data, address auto increments
	printWireError(Wire.endTransmission());
}

/**
 * Writes to LEDs. LEDs are in G, R, B order from the IC.
 * @param index: For single LED mode, this is the index of the LED. For RGB LED mode, this is the RGB LED group.
 * @param pwmRgb: The R, G, B values to write
 */
void IS31FL3246_LED_driver::writeRgb(uint8_t index, rgb8_t pwmRgb) {
	Wire.beginTransmission(_led_driver_address); // peripheral address
	if (_isRGB){
		index *=3;
	}
	Wire.write(HFP_L_DUTY_REG + index*HFP_REG_SIZE); // register address
	// data: HFP_L is 8 bits of PWM
	Wire.write(uint8_t(pwmRgb.g));
	// data: HFP_H is unused (7:4) Frequency mode select (3:2) HFP_H, 10-bit mode only (1:0)
	Wire.write(FREQUENCY_MODE_SELECT);
	Wire.write(uint8_t(pwmRgb.r));
	Wire.write(FREQUENCY_MODE_SELECT);
	Wire.write(uint8_t(pwmRgb.b));
	Wire.write(FREQUENCY_MODE_SELECT);
	printWireError(Wire.endTransmission());
}


/**
 *
 */
void IS31FL3246_LED_driver::update() {
	// call update register
	//slave address
	Wire.beginTransmission(_led_driver_address);
	// register address
	Wire.write(UPDATE_REG);
	// data
	Wire.write(UPDATE_PWM);
	printWireError(Wire.endTransmission());
}

/**
 *
 * @param err
 */
WireStatus::WireStatus IS31FL3246_LED_driver::printWireError(uint8_t err){
	// TODO: Turn off prints when not debugging
	  switch (err)
	  {
	    case WireStatus::SUCCESS:
//	      Serial.println("\tTransmitted without error.");
	      break;
	    case WireStatus::DATA_LENGTH_ERROR:
	      Serial.println("\tERROR: Data too long for buffer!!");
	      break;
	    case WireStatus::NACK_ADDRESS_ERROR:
	      Serial.print("\tERROR: Address 0x");
	      Serial.print(_led_driver_address, HEX);
		  Serial.println(" not acknowledged!!");
	      break;
	    case WireStatus::NACK_DATA_ERROR:
	      Serial.println("\tERROR: Data not acknowledged!!");
	      break;
	    case WireStatus::OTHER_ERROR:
	      Serial.println("\tERROR: Other error!!");
	      break;
	  }
	  return err;
}

