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
	SequencerDriver(IS31FL3246::IS31FL3246_LED_driver* pLedDriver);
	virtual ~SequencerDriver();
	void begin();
	void setBPM(uint8_t bpm);
	uint8_t getBPM();

	inline static uint8_t getSequencerCount() { return _sequencer_count; }
	static void (*const HANDLERS[N_SEQUENCERS])();
//	static void (*update)(); // TODO: Assign function such that all outputs update at once

	/** Setters and Getters */
	inline bool getStepFlag() { return stepFlag; }
	inline void clearStepFlag() { stepFlag = false; }
	inline IS31FL3246::rgb8_t getNextData(){ return _next_data; }
	inline void setNextData(IS31FL3246::rgb8_t next){ _next_data = next; }
	inline uint8_t getNextDataIndex(){ return _next_data_index; }
	inline void setNextDataIndex(uint8_t next){ _next_data_index = next; }

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
	IS31FL3246::IS31FL3246_LED_driver* _pLedDriver; // TODO: change to DAC
	// could also make this more extensible with a virtual parent class of led driver + DAC

	//data that can change
	const uint8_t _max_length = 12;
	uint8_t _bpm = 120;
	uint8_t _length = 0;

	// variables used in ISR;
	volatile bool stepFlag;
	volatile uint16_t* data = new uint16_t[_max_length];
	volatile uint8_t _next_data_index;
	volatile IS31FL3246::rgb8_t _next_data;


};

#endif /* SEQUENCERDRIVER_H_ */
