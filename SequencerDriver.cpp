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
	// 1 / (240 bpm / 60 s/min) - 1 / (239 bpm / 60 s/min) = 0.00105s = 952 Hz ~ 1kHz
	// divide by 10, we want 0.0001s tic time for our counters
	// And our lowest bpm is 40, corresponding to maximum time of
	// 60 s/min / 40 bpm = 1.5s = 1428.6 ticks @ 0.00105 ticks/s, so a int16_t should be large enough
	// compare match register = 16,000,000 Hz/ (prescalar * 1,000 Hz) -1, let prescalar = 64
	// compare match register = 16e6 Hz / (64  * 1e3 Hz) = 250


	// interrupt frequency (Hz) = (clock speed 16,000,000Hz) / (prescaler * (compare match register + 1))

	cli(); // Stop interrupts
	// Set timer1 interrupt
	TCCR1A = 0; // Clear register
	TCCR1B = 0; // Clear register
	TCNT1 = 0; // initialize counter value to 0
	// Set compare match register
	OCR1A = 250; //
	TCCR1B |= (1 << WGM12); // Turn on CTC? mode
//	TCCR1B |= (1 << CS12) | (1 << CS10); // Set 2 bits for 1024 prescalar
	TCCR1B |= (1 << CS11) | (1 << CS10); // Set 2 bits for 64 prescalar
	TIMSK1 |= (1 << OCIE1A); // Enable timer compare interrupt
	sei(); // Enable interrupts

	// Initialize gate pin as output, set low
	*gate_settings_port |= gate_mask;
	*gate_pin_port &= ~(gate_mask);

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

/**
 * Updates the output of a single sequencer. Changes CV out and turns gate on and off.
 * @return True if output step has changed
 */
bool SequencerDriver::updateOutput(void) {
	// If there is a new step, output voltage and turn on gate
	if (play_flag) {
		play_flag = false;
		// Update sequencer index
		this_index++;
		if (this_index >= length) {
			this_index = 0;
		}
		// Set outputs
		DacDriver->writeVout(dac_addr, getThisValue());
		*gate_pin_port |= gate_mask; // turn on gate
		return true;
	}
	// If tick count is above threshold, turn off gate
	else if (stop_flag) {
		stop_flag = false; // TODO: do we want to make it so stop flag is only enabled once per play flag?
		*gate_pin_port &= ~(gate_mask);
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
void SequencerDriver::step(void){
	tic_count++;
	if (tic_count >= max_tic_count) {
		tic_count = 0;
		play_flag = true;
	}
	else if (tic_count > tic_length_on) {
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

