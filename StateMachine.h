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
