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
SequencerDriver::SequencerDriver(void) {
	// TODO: perhaps linked list of sequencers so as one is removed, it's easy to add another
	// But doesn't really matter since sequencer driver objects won't really be dynamically created and removed
	_sequencer_id = _sequencer_count;
	_sequencer_count++;
	instances[_sequencer_id] = this;
}

/**
 * The destructor removes this sequencer from the list.
 * TODO: prevent dynamic creation of SequencerDrivers so this becomes irrelevant.
 */
SequencerDriver::~SequencerDriver(void) {
	instances[_sequencer_id] = NULL;
	_sequencer_count--;
}

/**
 *
 */
void SequencerDriver::begin(void){
	// Set up interrupts
	cli(); // Stop interrupts
	// Set timer1 interrupt
	TCCR1A = 0; // Clear register
	TCCR1B = 0; // Clear register
	TCNT1 = 0; // initialize counter value to 0
	// Set compare match register
	OCR1A = 15624; // 16,000,000/(1024*255) = 61 Hz TODO: are you sure????
	TCCR1B |= (1 << WGM12); // Turn on CTC? mode
	TCCR1B |= (1 << CS12) | (1 << CS10); // Set 2 bits for 1024 prescalar
	TIMSK1 |= (1 << OCIE1A); // Enable timer compare interrupt
	sei(); // Enable interrupts
}

/**
 * Code to execute in ISR.
 * TODO should probably run ISR at much faster rate and check for BPM
 * within ISR
 * TODO is next index needed?
 */
void SequencerDriver::step(void){
	_step_flag = true;
	_this_index = _next_index;
	_next_index++;
	if (_next_index >= _length) {
		_next_index = 0;
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



/** Definitions and initializers for static members */
SequencerDriver* SequencerDriver::instances[N_SEQUENCERS] = {};
uint8_t SequencerDriver::_sequencer_count = 0;

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

/**
 * Timer 1 interrupt service routine, which calls the handler for all sequencers.
 */
ISR(TIMER1_COMPA_vect) {
	for(uint8_t i=0; i < SequencerDriver::getSequencerCount(); i++) {
		SequencerDriver::HANDLERS[i]();
	}
}
