/*
 * IS31FL3246_LED_driver.cpp
 *
 *  Created on: Oct 18, 2022
 *      Author: Chris
 *	Usage:
 *		1. Instantiate class with constructor
 *		2. Call begin()
 *		3. Call writeRgb()
 *		4. Call update()
 */

#include "IS31FL3246_LED_driver.h"


/**
 * Implements addition operator that allows combining colors
 * @param a First color.
 * @param b Second color.
 * @return Added colors
 */
template <typename T> T operator+(const T a, const T b) {
	T c;
	if (UINT8_MAX - a.r > b.r) {
		c.r = UINT8_MAX;
	}
	else {
		c.r = a.r + b.r;
	}
	if (UINT8_MAX - a.g > b.g) {
		c.g = UINT8_MAX;
	}
	else {
		c.g = a.g + b.g;
	}
	if (UINT8_MAX - a.b > b.b) {
		c.b = UINT8_MAX;
	}
	else {
		c.b = a.b + b.b;
	}

	return c;
}

template rgb8_t operator+<rgb8_t>(rgb8_t a, rgb8_t b);
template rgb16_t operator+<rgb16_t>(rgb16_t a, rgb16_t b);
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
 * @param led_driver_address
 * @param sdb_pin
 * @param isRGB isRGB Denotes whether the driver controls 13 groups of 3(RGB) LEDs or 39 unassociated LEDs
 * @param is8bit
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
 * @return: The status of the transmission
 */
CommStatus IS31FL3246_LED_driver::begin() {
	CommStatus status = CommStatus::Success;
	Wire.begin();
	// setup IO pins
	pinMode(_sdb_pin, OUTPUT);
	// Set SDB pin high - enable LED driver
	digitalWrite(_sdb_pin, HIGH);
	Wire.beginTransmission(_led_driver_address); // slave address
	Wire.write(RESET_REG); // register address
	Wire.write(0xFF); // data
	status = printWireStatus(Wire.endTransmission());
	if(status != CommStatus::Success) {
		return status;
	}
	// Set control register: 8-bit mode, normal operation
	Wire.beginTransmission(_led_driver_address); // slave address
	Wire.write(CONTROL_REG); // register address
	Wire.write(uint8_t(~(1<<PMS_BIT) & (1<<SSD_BIT) )); // data: 8-bit mode, turn on normal operation
	status = printWireStatus(Wire.endTransmission());
	if(status != CommStatus::Success) {
		return status;
	}

	// Set global current
	rgb8_t globalCurrent = {GCCR, GCCG, GCCB};
	status = setRgbCurrent(globalCurrent);
	if(status != CommStatus::Success) {
		return status;
	}

	// Turn on LFP registers for all LEDs
	// TODO: any other registers that might need to be turned on for all LEDs?
	Wire.beginTransmission(_led_driver_address); // slave address
	Wire.write(LFP_DUTY_REG);
	for (int i = 0; i < N_LEDS; i++){
		Wire.write(0x80); // Low frequency PWM duty cycle
		// TODO: determine if LFP register introduces noise, if so,
		// change to 0xFF
	}
	status = printWireStatus(Wire.endTransmission());
	if(status != CommStatus::Success) {
		return status;
	}

	return status;
}


/**
 * Sets the global current for red LEDs
 * @param current: The global current for red LEDs
 * @return: The status of the transmission
 */
CommStatus IS31FL3246_LED_driver::setRCurrent(uint8_t current) {
	Wire.beginTransmission(_led_driver_address); // Device address
	Wire.write(GLOBAL_CURRENT_CTRL_REG_R); // Register address
	Wire.write(current); // data
	return printWireError(Wire.endTransmission());
}

/**
 * Sets the global current for green LEDs
 * @param current: The global current for green LEDs
 * @return: The status of the transmission
 */
CommStatus IS31FL3246_LED_driver::setGCurrent(uint8_t current) {
	Wire.beginTransmission(_led_driver_address); // slave address
	Wire.write(GLOBAL_CURRENT_CTRL_REG_G); // register address
	Wire.write(current); // data
	return printWireError(Wire.endTransmission());
}

/**
 * Sets the global current for blue LEDs
 * @param current: The global current for blue LEDs
 * @return: The status of the transmission
 */
CommStatus IS31FL3246_LED_driver::setBCurrent(uint8_t current) {
	Wire.beginTransmission(_led_driver_address); // slave address
	Wire.write(GLOBAL_CURRENT_CTRL_REG_B); // register address
	Wire.write(current); // data
	return printWireStatus(Wire.endTransmission());
}

/**
 * Sets the global current for red, green, and blue channels. Note that they are stored in G,R,B order
 * @param currentRgb: The current for all channels
 * @return: The status of the transmission
 */
CommStatus IS31FL3246_LED_driver::setRgbCurrent(rgb8_t currentRgb) {
	Wire.beginTransmission(_led_driver_address); // slave address
	Wire.write(GLOBAL_CURRENT_CTRL_REG_G); // register address
	Wire.write(currentRgb.g); // data
	Wire.write(currentRgb.r); // data, address auto increments
	Wire.write(currentRgb.b); // data, address auto increments
	return printWireStatus(Wire.endTransmission());
}

/**
 * Template function to write to LEDs. LEDs are in G, R, B order from the IC.
 * TODO: test this function for all template values
 * @tparam T
 * @param index: For single LED mode, this is the index of the LED. For RGB LED mode, this is the RGB LED group.
 * @param pwm: The PWM value to write
 * @return: The status of the transmission
 */
template <typename T> CommStatus IS31FL3246_LED_driver::writeLed(uint8_t index, T pwm) {
	Wire.beginTransmission(_led_driver_address); // peripheral address
	if (_isRGB){
		index *=3;
	}
	Wire.write(HFP_L_DUTY_REG + index*HFP_REG_SIZE); // register address
	writeData(pwm);
	return printWireStatus(Wire.endTransmission());
}

/**
 * Template function to write to multiple LEDs. LEDs are in G, R, B order from the IC.
 * TODO:
 * @tparam T
 * @param index: For single LED mode, this is the index of the LED. For RGB LED mode, this is the RGB LED group.
 * @param pPwm: The pointer to the PWM values to write
 * @param length: The length of the data to write
 * @return: The status of the transmission
 */
template <typename T> CommStatus IS31FL3246_LED_driver::writeConsecutiveLed(uint8_t index, T* pPwm, uint8_t length) {
	CommStatus wire_status = CommStatus::Success;
	int16_t writes_remaining = length;
	uint8_t write_size = 2;
	uint8_t writes_per_loop = BUFFER_LENGTH / 2;
	uint8_t start_idx = HFP_L_DUTY_REG + index*HFP_REG_SIZE;
	if (_isRGB){
		index *= 3; // R + G + B = iterating over indices 3x as fast as individual leds
		writes_per_loop /= 3;
		write_size = 6;
	}

	while (writes_remaining > 0) {
		Wire.beginTransmission(_led_driver_address); // peripheral address
		Wire.write(start_idx); // register address
		for(int i = 0; i < writes_per_loop; i++){
			writeData(*(pPwm+i));
		}
		wire_status = (CommStatus)Wire.endTransmission();
		if (wire_status != CommStatus::Success) {
			return printWireStatus(wire_status);
		}

		// Adjust indices for next loop
		pPwm += writes_per_loop;
		writes_remaining -= writes_per_loop;
		start_idx += writes_per_loop * write_size;
		if (writes_remaining < writes_per_loop) {
			writes_per_loop = writes_remaining;
		}
	}
	return printWireStatus(wire_status);
}


/**
 * Writing to update register is necessary to display the
 * @return: The status of the transmission
 */
CommStatus IS31FL3246_LED_driver::update() {
	// call update register
	//slave address
	Wire.beginTransmission(_led_driver_address);
	// register address
	Wire.write(UPDATE_REG);
	// data
	Wire.write(UPDATE_PWM);
	return printWireStatus(Wire.endTransmission());
}


/**
 * Overloaded function to write different single/RGB LED 8/10 bits of data
 * @param data
 */
void IS31FL3246_LED_driver::writeData(uint8_t data) {
	// data: HFP_L is 8 bits of PWM
	Wire.write(uint8_t(data));
	// data: HFP_H is unused (7:4) Frequency mode select (3:2) HFP_H, 10-bit mode only (1:0)
	Wire.write(FREQUENCY_MODE_SELECT);
}


/**
 * Overloaded function to write different single/RGB LED 8/10 bits of data
 * @param data
 */
void IS31FL3246_LED_driver::writeData(uint16_t data) {
	// data: HFP_L is 8 bits of PWM
	Wire.write(uint8_t(data));
	// data: HFP_H is unused (7:4) Frequency mode select (3:2) HFP_H, 10-bit mode only (1:0)
	Wire.write(FREQUENCY_MODE_SELECT | uint8_t(data >> 8));

}


/**
 * Overloaded function to write different single/RGB LED 8/10 bits of data
 * @param data
 */
void IS31FL3246_LED_driver::writeData(rgb8_t data) {
	// data: HFP_L is 8 bits of PWM
	Wire.write(uint8_t(data.g));
	// data: HFP_H is unused (7:4) Frequency mode select (3:2) HFP_H, 10-bit mode only (1:0)
	Wire.write(FREQUENCY_MODE_SELECT);
	Wire.write(uint8_t(data.r));
	Wire.write(FREQUENCY_MODE_SELECT);
	Wire.write(uint8_t(data.b));
	Wire.write(FREQUENCY_MODE_SELECT);
}


/**
 * Overloaded function to write different single/RGB LED 8/10 bits of data
 * @param data
 */
void IS31FL3246_LED_driver::writeData(rgb16_t data) {
	// data: HFP_L is 8 bits of PWM
	Wire.write(uint8_t(data.g));
	// data: HFP_H is unused (7:4) Frequency mode select (3:2) HFP_H, 10-bit mode only (1:0)
	Wire.write(FREQUENCY_MODE_SELECT | uint8_t(data.g >> 8));
	Wire.write(uint8_t(data.r));
	Wire.write(FREQUENCY_MODE_SELECT | uint8_t(data.r >> 8));
	Wire.write(uint8_t(data.b));
	Wire.write(FREQUENCY_MODE_SELECT | uint8_t(data.b >> 8));

}


// Template instantiation
template CommStatus IS31FL3246_LED_driver::writeLed<uint8_t>(uint8_t index, uint8_t pwm);
template CommStatus IS31FL3246_LED_driver::writeLed<uint16_t>(uint8_t index, uint16_t pwm);
template CommStatus IS31FL3246_LED_driver::writeLed<rgb8_t>(uint8_t index, rgb8_t pwm);
template CommStatus IS31FL3246_LED_driver::writeLed<rgb16_t>(uint8_t index, rgb16_t pwm);
template CommStatus IS31FL3246_LED_driver::writeConsecutiveLed<uint8_t>(uint8_t index, uint8_t* pPwm, uint8_t length);
template CommStatus IS31FL3246_LED_driver::writeConsecutiveLed<uint16_t>(uint8_t index, uint16_t* pPwm, uint8_t length);
template CommStatus IS31FL3246_LED_driver::writeConsecutiveLed<rgb8_t>(uint8_t index, rgb8_t* pPwm, uint8_t length);
template CommStatus IS31FL3246_LED_driver::writeConsecutiveLed<rgb16_t>(uint8_t index, rgb16_t* pPwm, uint8_t length);

