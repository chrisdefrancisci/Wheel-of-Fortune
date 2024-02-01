/*
 * RotaryEncoder.h
 *
 *  Created on: Jan 29, 2024
 *      Author: Chris
 */

#ifndef ROTARYENCODER_H_
#define ROTARYENCODER_H_

#include "Arduino.h"

const uint8_t enc_a = A2; // PC2
const uint8_t enc_b = 4;  // PD4
const uint8_t enc_sw_clock = 3; // PD3
constexpr uint8_t N_ROTARY_ENCODERS = 1;

enum class EncoderState : uint8_t {
	A_LOW_B_LOW = 0,
	A_LOW_B_HIGH = 1,
	A_HIGH_B_LOW = 2,
	A_HIGH_B_HIGH = 3
};

class RotaryEncoder {
	/**
	 * Using EC11E18244AU
	 *
	 * See https://tech.alpsalpine.com/e/products/detail/EC11E18244AU/
	 */
public:
	RotaryEncoder(uint8_t pin_a_, uint8_t pin_b_, uint8_t pin_clock_);
	virtual ~RotaryEncoder();

	static void (*const HANDLERS[N_ROTARY_ENCODERS])();
	static inline uint8_t getRotaryEncoderCount(){ return rotary_encoder_count; }

	void begin();
	void enable();
	void disable();
	int16_t readEncoder();
	uint8_t readButton();


private:
	// Management for interrupts across all/any instances
	// https://arduino.stackexchange.com/questions/65010/attachinterrupt-fail-to-initiate-inside-a-class
	// TODO maybe make base class for classes with interrupts?
	static RotaryEncoder* instances[N_ROTARY_ENCODERS];
	template <uint8_t ROTARY_ENCODER> static void handler() {
		if (instances[ROTARY_ENCODER] != NULL) { // TODO: prevent dynamic creation of RotaryEncoder so this becomes irrelevant.
			instances[ROTARY_ENCODER]->tic();
		}
	}
	void tic();
	static uint8_t rotary_encoder_count;

	uint8_t id;

	volatile int16_t pulse_count = 0;
	volatile EncoderState state = EncoderState::A_LOW_B_LOW;

	EncoderState getState();

	int16_t pulse_last = 0;
	uint8_t pin_a;
	uint8_t pin_b;
	uint8_t pin_clock;
};

#endif /* ROTARYENCODER_H_ */
