/*
 * Display.cpp
 *
 *  Created on: Nov 17, 2022
 *      Author: Chris
 */

#include "Display.h"

/** Maps the button index reported by Capacitive Button driver to the LED to feed to LED driver. */
const uint8_t button2led[N_CIRCLE_LEDS + N_PERIPHERAL_LEDS] =
		{0, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, // Circle indices
		2, 4, 3, 5, 6, 0, 7, 1}; // Peripheral indices

Display::Display() :
	circular_led_driver(circular_led_driver_address, sdb_pin, isRGB, is8bit),
	peripheral_led_driver(peripheral_led_driver_address, sdb_pin, isRGB, is8bit)
{
	displaySequencer(0); // TODO: maybe want to pull this into constructor?

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

	animation.rainbow_loop = 1;

	while (animation.rainbow_loop){
		update();
	}
	update();
}

/**
 * Turns off the entire set of circular LEDs.
 */
void Display::circleOff() {
	for (uint8_t ledIdx = 0; ledIdx < ledLen; ledIdx++){
		circular_led_driver.writeLed(ledIdx, OFF_RGB);
	}
	circular_led_driver.update();
}

/**
 * Turns off the entire set of peripheral LEDs.
 */
void Display::peripheralOff() {
	for (uint8_t ledIdx = 0; ledIdx < ledLen; ledIdx++){
		peripheral_led_driver.writeLed(ledIdx, OFF_RGB);
	}
	peripheral_led_driver.update();
}

/**
 * Illuminates the next LED in the sequence.
 * @param[in] sequencer_id The sequencer to grab the last index from.
 * @param[in] this_index The current index to write.
 */
void Display::step(uint8_t sequencer_id, uint8_t this_index) {
	sequencer_step[sequencer_id] = this_index;
}

/**
 *
 * @param sequencer
 * @param on
 */
void Display::displayGate(uint8_t sequencer, bool on) {
		rgb8_t color = OFF_RGB;
		sequencer = safe_sequencer(sequencer);

		if (on) {
			color = sequencerColors[sequencer];
		}
		if (sequencer == 0){
			peripheral_default[button2led[BUTTON_OUT_1]] = color;
		}
		else if (sequencer == 1){
			peripheral_default[button2led[BUTTON_OUT_2]] = color;
		}
		else if (sequencer == 2){
			peripheral_default[button2led[BUTTON_OUT_3]] = color;
		}
		else if (sequencer == 3){
			peripheral_default[button2led[BUTTON_OUT_4]] = color;
		}
		update_display = true;
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
		if (i % 12 == 0){
			Serial.print(" ");
		}
		Serial.print((uint8_t)pressedKeys[i]);
	}
	for (uint8_t i = 0; i < N_CIRCLE_LEDS; i++) {
		if (pressedKeys[i]) {
			circular_led_driver.writeLed(button2led[i], color);
			Serial.print("\tDisplay "); Serial.print(button2led[i]); Serial.print(" from "); Serial.print(i);
		}
		else {
			circular_led_driver.writeLed(button2led[i], OFF_RGB);
		}
	}
	Serial.println();
	circular_led_driver.update();
	// Turn on LEDs in second driver
	for (uint8_t i = 0; i < N_PERIPHERAL_LEDS; i++) {
		uint8_t complete_idx = i + N_CIRCLE_LEDS;
		if (pressedKeys[complete_idx]) {
			peripheral_led_driver.writeLed(button2led[complete_idx], color);
			Serial.print("\tDisplay "); Serial.print(button2led[complete_idx]); Serial.print(" from "); Serial.print(complete_idx);
		}
		else {
			peripheral_led_driver.writeLed(button2led[complete_idx], OFF_RGB);
		}
	}
	Serial.println();
	peripheral_led_driver.update();
}


/**
 * Toggles the play : pause LED in order to indicate recording in progress.
 * @param color The color to toggle the LED.
 * @param off True if LED should be turned off, as needed when exiting record mode. Defaults to false.
 */
void Display::recordingAnimation(rgb8_t color, bool off) {
	static uint32_t last_blink_time = 0;
	if (millis() > last_blink_time + RECORDING_BLINK_TIME) {
		last_blink_time = millis();
		static bool last_on = true; // Used to toggle every call. Initial value starts by turning it off.
		if (off || last_on) {
			last_on = false;
			peripheral_default[button2led[BUTTON_PLAY]] = OFF_RGB;
		}
		else {
			last_on = true;
			peripheral_default[button2led[BUTTON_PLAY]] = color;
		}
		update_display = true;
	}
}

/**
 * Creates a cute little rainbow animation. Non-blocking.
 * @param display_millis The time for each step to display
 * @return True if loop cycle has completed, otherwise false.
 */
void Display::rainbowLoopAnimation(unsigned long display_millis){
	static unsigned long last_millis = 0;
	unsigned long this_millis = millis();
	static int rgbIdx = 0;
	if (this_millis - last_millis >= display_millis) {
		last_millis = millis();

		for (int ledIdx = 0; ledIdx < ledLen; ledIdx++) {
			int rgbCircIdx = ((rgbIdx + ledIdx) >= rgbLen ) ? rgbIdx + ledIdx - rgbLen : rgbIdx + ledIdx;
			circular_default[button2led[ledIdx]] = rgbConsts[rgbCircIdx];
			if (ledIdx < N_PERIPHERAL_LEDS) {
				peripheral_default[button2led[N_CIRCLE_LEDS + ledIdx]] = WHITE_RGB;
			}
		}
		rgbIdx++;
		if (rgbIdx >= rgbLen) {
			rgbIdx = 0;
			animation.rainbow_loop = 0;
		}
		else {
			animation.rainbow_loop = 1;
		}
	}
	update_display = true;
}

void Display::setGateLengthAnimation(int16_t gate_length_) {
		update_display = true;
		if (gate_length_ > N_CIRCLE_LEDS) {
			gate_length = N_CIRCLE_LEDS;
		}
		else if (gate_length_ < 0) {
			gate_length = 0;
		}
		else {
			gate_length = gate_length_;
		}
		animation_start = millis();
		animation.gate_length = 1; // Set bitfield
	}

void Display::gateLengthAnimation() {
	uint32_t this_millis = millis();
	if ( this_millis - animation_start >= GATE_LENGTH_TIME) {
		animation.gate_length = 0;
		displayState(state); // Reset display back to state
	}
	else {
		for (int ledIdx = 0; ledIdx < ledLen; ledIdx++) {
			if (ledIdx < gate_length) {
				circular_default[button2led[ledIdx]] = WHITE_RGB;
			}
			else {
				circular_default[button2led[ledIdx]] = OFF_RGB;
			}
		}
//		update_display = true;
	}

}

void Display::bpmAnimation() {

}

/**
 *
 * @param button
 * @param color
 */
void Display::writeLed(ButtonMap_t button, rgb8_t color) {
	if (button <= BUTTON_STEP_11) {
		circular_led_driver.writeLed(button2led[button], color);
		circular_led_driver.update();
	}
	else {
		peripheral_led_driver.writeLed(button2led[button], color);
		peripheral_led_driver.update();
	}
}

/**
 *
 * @param state The current state - determines what the untouched display is.
 */
void Display::displayState(SequencerState state_) {
	// Force update when state is changed. In other cases (i.e., end of animation) update may or may not be desired
//	if (state != state_) {
//		update_display = true;
//	}
	update_display = true;
	state = state_;
//	animation.any = 0; // TODO: this seems like an ugly way to do this, display state not exclusive with animations

	if (state == SequencerState::Play) {
		for (uint8_t i = 0; i < N_CIRCLE_LEDS; i++) {
			circular_default[i] = OFF_RGB; // No button2led needed when all are the same
		}
		for (uint8_t i = 0; i < N_PERIPHERAL_LEDS; i++) {
			peripheral_default[i] = OFF_RGB;
		}

		animation.recording = 0;
	}
	else if (state == SequencerState::Stop) {
		for (uint8_t i = 0; i < N_CIRCLE_LEDS; i++) {
			// Display a black and white "keyboard", where sharp keys are black
			if (i == NOTE_i_s || i == NOTE_ii_s || i == NOTE_iv_s || i == NOTE_v_s || i == NOTE_vi_s) {
				circular_default[button2led[i]] = OFF_RGB; // button2led is needed because using button indices
			}
			else {
				circular_default[button2led[i]] = WHITE_RGB;
			}
		}
		for (uint8_t i = 0; i < N_PERIPHERAL_LEDS; i++) {
			peripheral_default[i] = OFF_RGB;
		}

		peripheral_default[button2led[BUTTON_PLAY]] = WHITE_RGB;

		// Reset sequencer_step to something that will not be displayed
		for (uint8_t i = 0; i < N_SEQUENCERS; i++) {
			sequencer_step[i] = UINT8_MAX;
		}

		animation.recording = 0;
	}
	else if (state == SequencerState::Record) {
		for (uint8_t i = 0; i < N_CIRCLE_LEDS; i++) {
			// Display a black and white "keyboard", where sharp keys are black
			if (i == NOTE_i_s || i == NOTE_ii_s || i == NOTE_iv_s || i == NOTE_v_s || i == NOTE_vi_s) {
				circular_default[button2led[i]] = OFF_RGB;
			}
			else {
				circular_default[button2led[i]] = WHITE_RGB;
			}
		}
		for (uint8_t i = 0; i < N_PERIPHERAL_LEDS; i++) {
			peripheral_default[i] = OFF_RGB;
		}

		// Have below block also be under sequencer selection, or disable sequencer selection while recording?
		if (active_sequencer == 0) {
			peripheral_default[button2led[BUTTON_OUT_1]] = sequencerColors[active_sequencer];
		}
		else if (active_sequencer == 1) {
			peripheral_default[button2led[BUTTON_OUT_2]] = sequencerColors[active_sequencer];
		}
		else if (active_sequencer == 2) {
			peripheral_default[button2led[BUTTON_OUT_3]] = sequencerColors[active_sequencer];
		}
		else if (active_sequencer == 3) {
			peripheral_default[button2led[BUTTON_OUT_4]] = sequencerColors[active_sequencer];
		}

		// Reset sequencer_step to something that will not be displayed
		for (uint8_t i = 0; i < N_SEQUENCERS; i++) {
			if (active_sequencer != i) {
				sequencer_step[i] = UINT8_MAX; // TODO: this should actually align with the current index being recorded to?
			}
		}

		animation.recording = 1;
	}
	else if (state == SequencerState::Error) {
		for (uint8_t i = 0; i < N_CIRCLE_LEDS; i++) {
			circular_default[i] = RED_RGB;
		}
		for (uint8_t i = 0; i < N_PERIPHERAL_LEDS; i++) {
			peripheral_default[i] = RED_RGB;
		}

		animation.recording = 0;
	}
}

/**
 * Determine final display and write it to the LED drivers.
 *
 * For both the circular LEDs and the peripheral LEDs, we copy the default buffer into the current buffer,
 * while checking for things like pressed keys, step changes, and animations.
 */
void Display::update() {
	// Resolve any animations
	if (animation.any){
		if (animation.recording){
			recordingAnimation(key_press_color);
		}
		if (animation.rainbow_loop) {
			rainbowLoopAnimation(100);
		}
		if (animation.gate_length) {
			gateLengthAnimation();
		}
		if (animation.bpm) {
			bpmAnimation();
		}
	}
	else {
		// Turn off any remaining animation effects
		recordingAnimation(key_press_color, true);
		// TODO: do I need to call displayState after this? Then I should save the state...
	}
	// Update display as needed
	if (!update_display) {
		return;
	}
	update_display = false;

	for (uint8_t i = 0; i < N_CIRCLE_LEDS; i++) {
		// Note that you can either iterate of i, or over button2led[i] but you CANNOT assign
		// to both in the same loop. You will end up overwriting things.
		uint8_t led_idx = button2led[i];
		circular_current[led_idx] = circular_default[led_idx];
		if (pressedKeys[i]) {
			// TODO maybe not display ring if touched during play state?
			circular_current[led_idx] = key_press_color;
		}
		// Map button representing current sequencer step step to LED
//		for (uint8_t j = 0; j < N_SEQUENCERS; j++) { // TODO when multiple sequencers are implemented
		for (uint8_t j = 0; j < 1; j++) {
			if (sequencer_step[j] == i) {
				circular_current[led_idx] = sequencerColors[j];
			}
		}
	}

	for (uint8_t i = 0; i < N_PERIPHERAL_LEDS; i++) {
		uint8_t button_idx = N_CIRCLE_LEDS + i;
		uint8_t led_idx = button2led[button_idx];
		peripheral_current[led_idx] = peripheral_default[led_idx];
		if (pressedKeys[button_idx]) {
			peripheral_current[led_idx] = key_press_color;
		}

	}
	circular_led_driver.writeConsecutiveLed(0, circular_current, N_CIRCLE_LEDS);
	peripheral_led_driver.writeConsecutiveLed(0, peripheral_current, N_PERIPHERAL_LEDS);

	circular_led_driver.update();
	peripheral_led_driver.update();
}
