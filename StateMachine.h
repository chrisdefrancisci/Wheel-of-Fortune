/*
 * StateMachine.h
 *
 *  Created on: Mar 25, 2023
 *      Author: Chris
 */

#ifndef STATEMACHINE_H_
#define STATEMACHINE_H_

#include "Arduino.h"

enum class SequencerState : uint8_t {
	Stop,
	Play,
	Record,
	// TODO: "Write" state, which would allow you to select note by index, then change gate length?
	//	Any other parameters we want to be allowed to be changed?
	Error
};

class StateMachine {
public:
	StateMachine();
	virtual ~StateMachine();
	inline SequencerState getState(void) { return state; };
	void setState(SequencerState nextState);
private:
	SequencerState state;

};

#endif /* STATEMACHINE_H_ */
