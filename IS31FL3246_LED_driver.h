/*
 * IS31FL3246_LED_driver.h
 *
 *  Created on: Oct 18, 2022
 *      Author: Chris
 */

#ifndef IS31FL3246_LED_DRIVER_H_
#define IS31FL3246_LED_DRIVER_H_

#include "Arduino.h"
#include "Wire.h"

namespace IS31FL3246 { // TODO may not be necessary or helpful
// constants

// Wire library constants
// End transmission return status
namespace WireStatus{
enum WireStatus : uint8_t {
SUCCESS = 0,
DATA_LENGTH_ERROR = 1, // data too long to fit in buffer
NACK_ADDRESS_ERROR = 2, // received NACK on transmit of address
NACK_DATA_ERROR = 3, // received NACK on transmit of data
OTHER_ERROR = 4
};
}


// LED Driver constants
const uint8_t N_LEDS = 24;

// Control register (*table 3, table 4 of datasheet)
const uint8_t CONTROL_REG = 0x00;
const uint8_t RGBM_BIT = 6; // individual LEDs when 0, grouped into RGB combinations when 1
const uint8_t RGB_GROUP_SIZE = 6; // used for address arithmetic - 3 LEDs x 2 bytes
const uint8_t HFPS_BIT_HIGH = 5; // 128kHz PWM when 1, see HFPS_BIT_LOW when 0
const uint8_t HFPS_BIT_LOW = 4; // 32kHz PWM when 0, 64 kHz PWM when 1
const uint8_t PMS_BIT = 1; // 8 bit mode when 0, 10 bit mode when 1 (HFP becomes 32kHz)
const uint8_t SSD_BIT = 0; // software shutdown when 0, normal operation when 1

// High Frequency PWM register
const uint8_t HFP_REG_SIZE = 2; // used for address arithmetic
// odd registers (0x01, 0x03, 0x05, ...)
const uint8_t HFP_L_DUTY_REG = 0x01; // high frequency pwm
const uint8_t HFP_L_MASK = B11111111; // HFP Low byte duty value (10-bit or 8-bit mode)
const uint8_t HFP_L_ON = 0xFF; // LED on
const uint8_t HFP_L_OFF = 0x00; // LED off
// even registers (0x02, 0x04, 0x06, ...)
const uint8_t HFP_H_DUTY_REG = 0x02; // high frequency pwm
const uint8_t FMS_BIT_HIGH = 3; // frequency mode select: HFP + LFP when 00, only HFP/LFP=256 when 01
const uint8_t FMS_BIT_LOW = 2; // no PWM/output always on when 10, channel shutdown when 11
const uint8_t HFP_H_MASK = B00000011; // HFP High byte duty value (10-bit mode only, set to 0 o.w.)
const uint8_t HFP_H_ON = 0x03; // LED on
const uint8_t HFP_H_OFF = 0x00; // LED off

// Low Frequency PWM register, operates at 127 Hz
const uint8_t LFP_DUTY_REG = 0x49; // low frequency pwm, 0-255

// Update register
// write 0x00 to update register when SDB pin="H" and SSD="1" to update PWM registers (0x01~0x6C)
const uint8_t UPDATE_REG = 0x6D; // updates HFP and LFP data
const uint8_t UPDATE_PWM = 0x00; // write this to update register to update PWM

// Global current registers (*tables 9~11 in datasheet)
// modulates all channels DC current, default 0x00
const uint8_t GLOBAL_CURRENT_CTRL_REG_G = 0x6E; // global current of all green channels
const uint8_t GLOBAL_CURRENT_CTRL_REG_R = 0x6F; // global current of all red channels
const uint8_t GLOBAL_CURRENT_CTRL_REG_B = 0x70; // global current of all blue channels

// Phase delay and clock register (*table 12 in datasheet)
const uint8_t PHASE_DELAY_CLOCK_PHASE_REG = 0x71; // phase delay and clock phase
const uint8_t PDE_BIT = 7; // phase delay disabled when 0, enabled when 1 (see figure 11)
const uint8_t HLS_BIT = 6; // 6 group phase delay at LFP when 0, delay at HFP when 1
const uint8_t CLOCK_PHASE_6 = B00100000; // for each of the 6 groups of phase delay,
const uint8_t CLOCK_PHASE_5 = B00010000; // the clock phase option can be enabled
const uint8_t CLOCK_PHASE_4 = B00001000; // when set to 1 (see figure 13)
const uint8_t CLOCK_PHASE_3 = B00000100;
const uint8_t CLOCK_PHASE_2 = B00000010;
const uint8_t CLOCK_PHASE_1 = B00000001;

// Reset register
const uint8_t RESET_REG = 0x7F; // reset all registers when set to 0x00

// Settings selections
const uint8_t FREQUENCY_MODE_SELECT = uint8_t( ~(1<<FMS_BIT_HIGH) & (1<<FMS_BIT_LOW));

// Test LEDs built in to dev board
//const uint8_t LED_DRIVER_ADDRESS =  B0110000; // this assumes AD connected to GND
//const uint8_t RGB_GROUP = 11; // note that in datasheet groups are "1-indexed" where here the groups
// are zero indexed (this would be group 12 in the datasheet)
const uint8_t GCCG = 0x01; // start with global current control at fraction of max
const uint8_t GCCR = 0x04;
const uint8_t GCCB = 0x04;

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} rgb8_t;

typedef struct {
	uint16_t r;
	uint16_t g;
	uint16_t b;
} rgb16_t;

//class rgbGroup {
//public:
//	rgbGroup(int val);
//	uint8_t value();
//private:
//	const int min = 0;
//	const int max = 12;
//	uint8_t _val;
//};

class IS31FL3246_LED_driver {
public:
	IS31FL3246_LED_driver(uint8_t led_driver_address, uint8_t sdb_pin, bool isRGB, bool is8bit);
	virtual ~IS31FL3246_LED_driver();

	WireStatus::WireStatus begin();

	// TODO: could simplify code by utilizing templates
	void setRCurrent(uint8_t current);
	void setGCurrent(uint8_t current);
	void setBCurrent(uint8_t current);
	void setRgbCurrent(rgb8_t currentRgb);

	void writeRgb(uint8_t addr, rgb8_t pwmRgb);
	void writeRgb(uint8_t addr, rgb16_t pwmRgb);
	void writeConsecutiveRgb(uint8_t addr, rgb8_t* pwmRgb, uint8_t length);
	void writeConsecutiveRgb(uint8_t addr, rgb16_t* pwmRgb, uint16_t length);
	void writeLed(uint8_t addr, uint8_t pwm);
	void writeLed(uint8_t addr, uint16_t pwm);
	void writeConsecutiveLed(uint8_t addr, uint8_t pwm, uint8_t length);
	void writeConsecutiveLed(uint8_t addr, uint16_t pwm, uint8_t length);

	void update();

private:
	const uint8_t _led_driver_address;
	const uint8_t _sdb_pin;
	const bool _isRGB;
	const bool _is8bit;
	WireStatus::WireStatus printWireError(uint8_t err);
};

}
#endif /* IS31FL3246_LED_DRIVER_H_ */
