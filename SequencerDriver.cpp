/*
 * SequencerDriver.cpp
 *
 *  Created on: Nov 17, 2022
 *      Author: Chris
 */

#include "SequencerDriver.h"

/**
 * Constructor increments the count of existing sequencers and adds this sequencer to the array.
 * This allows the ISR to run the handler for this sequencer.
 */
SequencerDriver::SequencerDriver(AD5695* dac_driver) :
	DacDriver(dac_driver)
{
	// TODO: perhaps linked list of sequencers so as one is removed, it's easy to add another
	// But doesn't really matter since sequencer driver objects won't really be dynamically created and removed
	sequencer_id = sequencer_count;
	sequencer_count++;
	instances[sequencer_id] = this;
	// Assign gate I/O pin based on sequencer ID
//	gate_settings_port = pGateSetupArray[sequencer_id];
//	Serial.println("Constructor!!!");
	gate_settings_port = pGateSetupArray[sequencer_id];
	gate_pin_port = pGatePortArray[sequencer_id];
	gate_mask = gateArray[sequencer_id];
//	Serial.print("sequencer_id = "); Serial.println(sequencer_id);
//	Serial.print("Gate pin port = 0x"); Serial.print((uint8_t)gate_pin_port, HEX); Serial.print(" vs pGatePortArray[0] = 0x");
//	Serial.println((uint8_t)pGatePortArray[0], HEX);
	for (uint8_t i = 0; i < max_length; i++) {
		setGateLength(i, i+1); // TODO: should probably initialize to something more reasonable, like 11 or 0.
	}
}

/**
 * The destructor removes this sequencer from the list.
 * TODO: prevent dynamic creation of SequencerDrivers so this becomes irrelevant.
 */
SequencerDriver::~SequencerDriver(void) {
	instances[sequencer_id] = NULL;
	sequencer_count--;
}

/**
 * Initializes timers
 * TODO: does it matter if timers are initialized for every instance?
 */
void SequencerDriver::begin(void){
	// Set up interrupts
	// Not exactly sure if this is the right way to think about setting interrupt time, but
	// If our max allowable bpm is 240, and we want to have 10 "tics" of resolution between 239
	// and 240, then
	// 1 / (240 bpm / 60 s/min) - 1 / (239 bpm / 60 s/min) = 0.00105s = 952 Hz minimum, let's use 1kHz
	// divide by 10, we want 0.0001s tic time for our counters -> 10 kHz resolution
	// And our lowest bpm is 40, corresponding to maximum time of
	// 60 s/min / 40 bpm = 1.5s = 15000 ticks @ 0.0001 ticks/s, so a int16_t should be large enough

	// compare match register = 16,000,000 Hz/ (prescalar * 1,000 Hz) -1, let prescalar = 64
	// compare match register = 16e6 Hz / (64  * 1e4 Hz) - 1 = 24

	// interrupt frequency (Hz) = (clock speed 16,000,000Hz) / (prescaler * (compare match register + 1))

	cli(); // Stop interrupts
	// Set timer1 interrupt
	TCCR1A = 0; // Clear register
	TCCR1B = 0; // Clear register
	TCNT1 = 0; // initialize counter value to 0
	// Set compare match register
	OCR1A = 24; //
	TCCR1B |= (1 << WGM12); // Turn on CTC? mode
//	TCCR1B |= (1 << CS12) | (1 << CS10); // Set 2 bits for 1024 prescalar
	TCCR1B |= (1 << CS11) | (1 << CS10); // Set 2 bits for 64 prescalar
	TIMSK1 |= (1 << OCIE1A); // Enable timer compare interrupt
	sei(); // Enable interrupts

	// Initialize gate pin as output, set low
//	*gate_settings_port |= gate_mask;
//	*gate_pin_port &= ~gate_mask;
	*pGate0Setup |= gate0; // Set to output (1)
	*pGate0Port &= ~gate0; // Set low

}

/**
 * Sets sequencer BPM.
 *
 * Setting the BPM requires updating the number of tics the timing interrupt should occur,
 * stored in max_tic_count. This follows the function:
 *
 * max_tic_count = (60 s / min) / BPM * 10 kHz
 *
 * Where 10kHz comes from setting the prescalar and compare match register in SequencerDriver::begin().
 *
 * @param bpm Beats per minute
 */
void SequencerDriver::setBPM(uint8_t bpm_) {
	// Enforce arbitrary min and max BPM
	if (bpm_ < 40) {
		bpm = 40;
	}
	else if (bpm_ > 240) {
		bpm = 240;
	}
	else {
		bpm = bpm_;
	}
	uint16_t last_max_tic_count = max_tic_count;
	max_tic_count = uint16_t(60.0 *  10e3 / float(bpm));
	float tic_ratio = float(max_tic_count) / float(last_max_tic_count);

	tic_count = 0;
	play_flag = true;
	stop_flag = false;
	for (uint8_t index = 0; index < max_length; index ++){
		Serial.print("Tic length going from "); Serial.print(data[index].tic_length);
		data[index].tic_length = uint16_t(float(data[index].tic_length) * tic_ratio);
		Serial.println(" to "); Serial.println(data[index].tic_length);
	}
}

/**
 * Converts note index to DAC value
 * @param[in] note Index, from 0 to NOTES2DAC_LEN
 * @return The value to write to the DAC, corresponding to the given note
 */
uint16_t SequencerDriver::getDacValue(uint8_t note) {
	if (note >= NOTES2DAC_LEN) {
		note = NOTES2DAC_LEN;
	}
	return pgm_read_word_near(NOTES2DAC + note);
}

/**
 * Converts note index to DAC value
 * @param[in] note Index, from 0 to NOTES2DAC_LEN - 1
 * @param[in] octave Index, from 0 to N_OCTAVES - 1
 * @return The value to write to the DAC, corresponding to the given note and octave.
 */
uint16_t SequencerDriver::getDacValue(uint8_t note, uint8_t octave) {
	uint8_t index = note + octave * OCTAVES_2_NOTES;
	if (index >= NOTES2DAC_LEN) {
		index = NOTES2DAC_LEN;
	}
	return pgm_read_word_near(NOTES2DAC + index);
}

void SequencerDriver::setGateLength(uint8_t index, uint8_t gate_length_fraction) {
	// Buttons are on 0 - 11 scale, but articulation will be on 1 - 12 scale, with 0 set using other I/O.
	if (gate_length_fraction > BUTTON_STEP_11 + 1) {
		gate_length_fraction = BUTTON_STEP_11 + 1;
	}
	if (gate_length_fraction == 0) {
		data[index].tic_length = 0;
	}
	else {
		data[index].tic_length = max_tic_count * gate_length_fraction / (BUTTON_STEP_11 + 1);
	}
}

/**
 * Updates the output of a single sequencer. Changes CV out and turns gate on and off.
 * @return True if output step has changed
 */
bool SequencerDriver::updateOutput(SequencerData &data) {
	// If there is a new step, output voltage and turn on gate
	if (play_flag) {
		play_flag = false;
		// Update sequencer index
		this_index++;
		if (this_index >= length) {
			this_index = 0;
		}
		// Set outputs
		DacDriver->writeVout(dac_addr, getThisPitch());
		tic_length_on = getThisGateLength();
//		*gate_pin_port |= gate_mask; // turn on gate
		if (tic_length_on > 0) {
			gateOn();
		}
		data.step = this_index;
		data.gate_on = true;
		return true;
	}
	// If tick count is above threshold, turn off gate
	else if (stop_flag) {
		stop_flag = false; // TODO: do we want to make it so stop flag is only enabled once per play flag?
//		*gate_pin_port &= ~(gate_mask);
		gateOff();
		data.step = this_index;
		data.gate_on = false;
		return true;
	}
	return false;
}

/* Private Functions --------------------------------------------------------*/

/**
 * Code to execute in ISR.
 * TODO should probably run ISR at much faster rate and check for BPM
 * within ISR
 * TODO is next index needed?
 */
void SequencerDriver::tic(void){
	tic_count++;
	if (tic_count >= max_tic_count) {
		tic_count = 0;
		play_flag = true;
	}
	else if (tic_count == tic_length_on) {
		stop_flag = true;
	}
}

/* ISRs ---------------------------------------------------------------------*/

/**
 * Timer 1 interrupt service routine, which calls the handler for all sequencers.
 */
ISR(TIMER1_COMPA_vect) {
	for(uint8_t i=0; i < SequencerDriver::getSequencerCount(); i++) {
		SequencerDriver::HANDLERS[i]();
	}
}

/**
 * Array containing handler functions for calling within interrupts.
 */
void (*const SequencerDriver::HANDLERS[N_SEQUENCERS])(void) =
{
		SequencerDriver::handler<0>,
		SequencerDriver::handler<1>,
		SequencerDriver::handler<2>,
		SequencerDriver::handler<3>,
};

/* Definitions and initializers for static members --------------------------*/
SequencerDriver* SequencerDriver::instances[N_SEQUENCERS] = {};
uint8_t SequencerDriver::sequencer_count = 0;

