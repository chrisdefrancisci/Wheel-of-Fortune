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
#include "StateMachine.h"

#include "Arduino.h"
#include "Bitfield.h"

// Colors
const rgb8_t OFF_RGB = {0, 0, 0};
const rgb8_t WHITE_RGB = {180, 180, 180};

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
const uint8_t N_CIRCLE_LEDS = 12;
const uint8_t N_PERIPHERAL_LEDS = 8;
const uint8_t circular_led_driver_address = B0110000; // AD connected to GND
const uint8_t peripheral_led_driver_address = B0110011; // AD connected to +5V
const uint8_t sdb_pin = 9;
const bool isRGB = true;
const bool is8bit = true;
const uint8_t ledLen = 12;

const uint16_t RECORDING_BLINK_TIME = 500; /** ms, the amount of time on/off for recording blinking animation */

union buttons { // TODO
	rgb8_t array[N_CIRCLE_LEDS + N_PERIPHERAL_LEDS];
	struct {
		rgb8_t a;
	};
};

enum class Animation : uint8_t {
	None,
	RainbowLoop,
	Recording
};

class Display {
public:
	Display();
	virtual ~Display();

	void begin();
	void rainbowLoopAnimation(unsigned long display_millis);
	void recordingAnimation(rgb8_t color = WHITE_RGB, bool off = false);
	void circleOff();
	void peripheralOff();


	void displayState(SequencerState state);
	void displayPressedKeys(Bitfield<QT1245_DETECT_BYTES> pressedKeys, rgb8_t color); // TODO deprecated
	/** Set current pressed keys. */
	inline void displayAllPressedKeys(Bitfield<QT1245_DETECT_BYTES> pressedKeys_) {
		pressedKeys = pressedKeys_;
		update_display = true;
	}

	/**
	 * Sets the current sequencer so that color is used for button presses
	 * @param sequencer The index of the sequencer
	 */
	inline void displaySequencer(uint8_t sequencer) {
		active_sequencer = safe_sequencer(sequencer);
		key_press_color = sequencerColors[active_sequencer];
		update_display = true;
	}

	/**
	 *
	 * @param sequencer
	 * @param step
	 */
	inline void displayStep(uint8_t sequencer, uint8_t step) {
		sequencer = safe_sequencer(sequencer);
		sequencer_step[sequencer] = step;
		update_display = true;
	}

	void displayGate(uint8_t sequencer, bool on);
	void color(rgb8_t color); // TODO
	void color(uint8_t sequencer_id); // TODO
	void step(uint8_t sequencer_id, uint8_t this_index);
	void update(); // TODO
	void writeLed(ButtonMap_t button, rgb8_t color);

private:

	inline uint8_t safe_sequencer(uint8_t seq) {
		if (seq >= N_SEQUENCERS) {
			// unsigned int, so it will never be less than 0
			seq = N_SEQUENCERS - 1;
		}
		return seq;
	}

	IS31FL3246_LED_driver circular_led_driver; // Driver for LEDs in a circle, with address b0110 000x
	IS31FL3246_LED_driver peripheral_led_driver; // Driver for LEDs in the corner, with address b0110 011x
	uint8_t last_LED[N_SEQUENCERS]; // Keep track of which LEDs need to be cleared on the next sequencer step
	// TODO: What data structure to use to hold current display for clearing? I think I could probably get rid of the "last_LED" stuff
	// Instead, just have a "current default" as a class member and "modifications" passed via a function
	uint8_t sequencer_step[N_SEQUENCERS] = { UINT8_MAX };
	bool update_display = false;
	bool sequencer_gate[N_SEQUENCERS] = { false };
	rgb8_t circular_default[N_CIRCLE_LEDS];
	rgb8_t peripheral_default[N_PERIPHERAL_LEDS];
	rgb8_t circular_current[N_CIRCLE_LEDS]; // TODO determine if needed
	rgb8_t peripheral_current[N_PERIPHERAL_LEDS]; // TODO determine if needed
	rgb8_t key_press_color = WHITE_RGB;
	uint8_t active_sequencer = 0;
	Bitfield<QT1245_DETECT_BYTES> pressedKeys;
	Animation animation = Animation::None;


};



#endif /* DISPLAY_H_ */
