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
#include "IS31FL3246_LED_driver.h"

constexpr uint8_t N_SEQUENCERS = 4; // 4 sequencers, for 4 outputs

class SequencerDriver {
public:
	SequencerDriver();
	virtual ~SequencerDriver();
	void begin();
	void setBPM(uint8_t bpm);
	uint8_t getBPM();

	inline static uint8_t getSequencerCount() { return _sequencer_count; }
	static void (*const HANDLERS[N_SEQUENCERS])();
//	static void (*update)(); // TODO: Assign function such that all outputs update at once

	/** Setters and Getters */
	inline bool getStepFlag() { return _step_flag; }
	inline void clearStepFlag() { _step_flag = false; }
	inline uint8_t getThisIndex(){ return _this_index; }
	inline uint8_t getNextIndex(){ return _next_index; }
	inline uint8_t getMaxLength() {return _max_length;}
	inline uint8_t getSequenceLength() {return _length;}
	inline void setSequenceLength(uint8_t len) { _length = len;}

private:

	// Management of SequencerDriver across all instances
	// this is from this tutorial:
	// https://arduino.stackexchange.com/questions/65010/attachinterrupt-fail-to-initiate-inside-a-class
	static SequencerDriver* instances[N_SEQUENCERS];

	template <uint8_t SEQUENCER> static void handler()
	{
		instances[SEQUENCER]->step();
	}
	void attach();
	void step();
	static uint8_t _sequencer_count;
	uint8_t _sequencer_id;

	// instance specific
	// could also make this more extensible with a virtual parent class of led driver + DAC

	//data that can change
	const uint8_t _max_length = 12;
	uint8_t _bpm = 120;

	uint16_t* data = new uint16_t[_max_length];

	// variables used in ISR;
	volatile bool _step_flag;
	volatile uint8_t _length = 12;
	volatile uint8_t _this_index = _length-1;
	volatile uint8_t _next_index = 0;


};

#endif /* SEQUENCERDRIVER_H_ */
