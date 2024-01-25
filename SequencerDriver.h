/*
 * SequencerDriver.h
 *
 *  Created on: Nov 17, 2022
 *      Author: Chris
 *  Writes data to I2C address
 */

#ifndef SEQUENCERDRIVER_H_
#define SEQUENCERDRIVER_H_

#include "Arduino.h"
#include "Hardware.h"
#include "AD5695.h"
#include "IS31FL3246_LED_driver.h"
#include "DAC_14_bit_notes.h"

constexpr uint8_t N_SEQUENCERS = 4; // 4 sequencers, for 4 outputs
constexpr uint8_t N_OCTAVES = 5; // 5 octave range
constexpr uint8_t OCTAVES_2_NOTES = 11;

typedef struct SequencerData {
	uint8_t step = UINT8_MAX;
	bool gate_on = false;
} SequencerData_t;

typedef struct NoteData {
	/** The note value, converted to DAC using DAC_14_bit_notes.h */
	uint16_t dac_value = 0;
	/** The gate length, converted into interrupt tics */
	uint16_t tic_length = 0;
} NoteData_t;

class SequencerDriver {
public:
	SequencerDriver(AD5695* dac_driver);
	virtual ~SequencerDriver();
	void begin();
	void setBPM(uint8_t bpm_);
	inline uint8_t getBPM(){ return bpm; };

	inline static uint8_t getSequencerCount() { return sequencer_count; }
	static void (*const HANDLERS[N_SEQUENCERS])();
//	static void (*update)(); // TODO: Assign function such that all outputs update at once
	bool updateOutput(SequencerData &data);

	uint16_t getDacValue(uint8_t note);
	uint16_t getDacValue(uint8_t note, uint8_t octave);

	// Setters and Getters
	inline uint8_t getThisIndex(){ return this_index; }
	inline void restartIndex(){ this_index = 0; }
	inline void incrementIndex(){ this_index = (this_index < length - 1) ? this_index + 1 : 0; }
	inline uint8_t getMaxLength() {return max_length;}
	inline uint8_t getSequenceLength() {return length;}
	inline void setSequenceLength(uint8_t len) { length = len;}
	inline uint8_t getId() { return sequencer_id; } // I don't think this is useful??
	inline void setPitch(uint8_t index, uint8_t pitch){ data[index].dac_value = getDacValue(pitch); }
	inline void setThisPitch(uint8_t pitch){ data[this_index].dac_value = getDacValue(pitch); }
	void setGateLength(uint8_t index, uint8_t gate_length_fraction);
	void setThisGateLength(uint8_t gate_length_fraction){ setGateLength(this_index, gate_length_fraction); };
	inline uint16_t getThisPitch(){ return data[this_index].dac_value; }
	inline uint16_t getThisGateLength(){ return data[this_index].tic_length; }
	inline void incrementOctave(){ if(octave < N_OCTAVES - 1) octave++; }
	inline void decrementOctave(){ if(octave > 0) octave--; }
	uint8_t getOctave(){ return octave; }
	void gateOn(){ *(gate_pin_port) |= gate_mask; }
	void gateOff(){ *(gate_pin_port) &= ~gate_mask; }


private:
	// Management of SequencerDriver across all instances
	// this is from this tutorial:
	// https://arduino.stackexchange.com/questions/65010/attachinterrupt-fail-to-initiate-inside-a-class
	static SequencerDriver* instances[N_SEQUENCERS];

	template <uint8_t SEQUENCER> static void handler() {
		if (instances[SEQUENCER] != NULL) { // TODO: prevent dynamic creation of SequencerDrivers so this becomes irrelevant.
			instances[SEQUENCER]->tic();
		}
	}
	void tic();
	static uint8_t sequencer_count;
	static constexpr uint8_t max_length = 12;



	// instance specific
	AD5695* DacDriver;
	// TODO: have these be determined in constructor, perhaps given pin that corresponds to the sequencer
	DacAddr dac_addr = DacAddr::DACA;
	uint8_t gate_mask = 1 << 5; // Gate output mask within port
	volatile uint8_t* gate_pin_port;// = &PORTD; // Port corresponding setting gate pin
	volatile uint8_t* gate_settings_port;// = &DDRD; // Settings register


	// data that can change
	uint8_t sequencer_id;
	uint8_t bpm = 120;

	NoteData data[max_length];
	uint8_t octave = 0;
	uint8_t length = 12;
	uint8_t this_index = length-1;

	// variables used in ISR;
	volatile bool play_flag = true;
	volatile bool stop_flag = false;
	/// Number of times interrupt is hit
	volatile uint16_t tic_count = 0;
	/// Number of interrupt hits that correspond to gate on / note playing (tic_length_on) / (max_tic_count) is
	/// gate length from 0 to 1
	volatile uint16_t tic_length_on = 250;
	/// Number of interrupt hits for each step. max_tic_count = 60 s/min / bpm * 10kHz
	/// Ex: bpm = 120 -> 0.5s, interrupt freq = 10kHz = 0.0001s, max_tic_count = 60 s/min / 120bpm * 10kHz = 5000 ticks
	volatile uint16_t max_tic_count = 5000;


};

#endif /* SEQUENCERDRIVER_H_ */
