/*
 * RotaryEncoder.cpp
 *
 *  Created on: Jan 29, 2024
 *      Author: Chris
 */

#include "RotaryEncoder.h"

void func(){
	Serial.println("In interrupt");
}

RotaryEncoder::RotaryEncoder(uint8_t pin_a_, uint8_t pin_b_, uint8_t pin_clock_) :
	pin_a(pin_a_),
	pin_b(pin_b_),
	pin_clock(pin_clock_)
{
	// TODO Auto-generated constructor stub
	id = rotary_encoder_count;
	rotary_encoder_count++;
	instances[id] = this;

}

RotaryEncoder::~RotaryEncoder() {
	// TODO Auto-generated destructor stub
	instances[id] = NULL;

}

/**
 * TODO setup pin interrupts
 */
void RotaryEncoder::begin() {
	// Set up interrupts on pin a and pin b, maybe pin c
	pinMode(pin_a, INPUT);
	pinMode(pin_b, INPUT);
	pinMode(pin_clock, INPUT);

	// From the datasheet, I think we don't need an interrupt on the B signal
	state = getState();
//	attachInterrupt(digitalPinToInterrupt(pin_a), RotaryEncoder::HANDLERS[0], CHANGE); // TODO do this in the not-arduino way
	PCICR |= 0b00000010;  // Turn on pin change interrupts on port c
	PCMSK1 |= 0b00000100; // Turn on interrupt for PC2
}

/**
 * Enable interrupts and rotary encoder usage.
 */
void RotaryEncoder::enable() {

}

/**
 * Disable interrupts and rotary encoder usage.
 */
void RotaryEncoder::disable() {

}

/**
 * TODO implement this so it's not just nonsense
 * @return
 */
int16_t RotaryEncoder::readEncoder() {
	int16_t pulse_diff = pulse_count - pulse_last;
	pulse_last = pulse_count;
	return pulse_diff;
}

/* Private Functions --------------------------------------------------------*/
EncoderState RotaryEncoder::getState() {
	uint8_t state_int = (digitalRead(pin_a) << 1) + digitalRead(pin_b);
	if (state_int == 0) {
		return EncoderState::A_LOW_B_LOW;
	}
	else if (state_int == 1) {
		return EncoderState::A_LOW_B_HIGH;
	}
	else if (state_int == 2) {
		return EncoderState::A_HIGH_B_LOW;
	}
	else if (state_int == 3) {
		return EncoderState::A_HIGH_B_HIGH;
	}
	else {
		return EncoderState::A_LOW_B_LOW;
	}
}

void RotaryEncoder::tic() {
	/**
	 * This implements the full truth table of states for a[old] b[old] a[new] b[new].
	 *
	 * old | new |
	 * a b | a b | output
	 * ----|-----|-------------------
	 * 0 0 | 0 0 | none
	 * 0 0 | 0 1 | illegal
	 * 0 0 | 1 0 | clockwise
	 * 0 0 | 1 1 | counter-clockwise
	 * 0 1 | 0 0 | illegal
	 * 0 1 | 0 1 | none
	 * 0 1 | 1 0 | clockwise
	 * 0 1 | 1 1 | counter-clockwise
	 * 1 0 | 0 0 | counter-clockwise
	 * 1 0 | 0 1 | clockwise
	 * 1 0 | 1 0 | none
	 * 1 0 | 1 1 | illegal
	 * 1 1 | 0 0 | counter-clockwise
	 * 1 1 | 0 1 | clockwise
	 * 1 1 | 1 0 | illegal
	 * 1 1 | 1 1 | none
	 *
	 * I.e., in nominal cases, only the end value matters. [10] and [01] indicate clockwise,
	 * while [00] and [11] indicate counter-clockwise. However, we will ignore any illegal transitions.
	 */
	EncoderState old_state = state;
	state = getState();
	switch (state) {
	case EncoderState::A_LOW_B_LOW:
		// Counter-clockwise except for illegal combo
		if (old_state == EncoderState::A_LOW_B_HIGH){
			break;
		}
		pulse_count--;
		break;

	case EncoderState::A_LOW_B_HIGH:
		// Clockwise except for illegal combo
		if (old_state == EncoderState::A_LOW_B_LOW){
			break;
		}
		pulse_count++;
		break;

	case EncoderState::A_HIGH_B_LOW:
		// Clockwise except for illegal combo
		if (old_state == EncoderState::A_HIGH_B_HIGH){
			break;
		}
		pulse_count++;
		break;

	case EncoderState::A_HIGH_B_HIGH:
		// Counter-clockwise except for illegal combo
		if (old_state == EncoderState::A_HIGH_B_LOW){
			break;
		}
		pulse_count--;
		break;
	default:
		break;
	}
}

/* ISRs ---------------------------------------------------------------------*/

/**
 * Pin change on port C interrupt service routine, which calls the handler for all sequencers.
 */
ISR(PCINT1_vect) {
	for(uint8_t i=0; i < RotaryEncoder::getRotaryEncoderCount(); i++) {
		RotaryEncoder::HANDLERS[i]();
	}
}

/**
 * Array containing handler functions for calling within interrupts.
 */
void (*const RotaryEncoder::HANDLERS[N_ROTARY_ENCODERS])(void) =
{
		RotaryEncoder::handler<0>
};

/* Definitions and initializers for static members --------------------------*/
RotaryEncoder* RotaryEncoder::instances[N_ROTARY_ENCODERS] = {};
uint8_t RotaryEncoder::rotary_encoder_count = 0;
