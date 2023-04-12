/*
 * StateMachine.cpp
 *
 *  Created on: Mar 25, 2023
 *      Author: Chris
 */

#include "StateMachine.h"

StateMachine::StateMachine() {
	state = SequencerState::Stop;
}

StateMachine::~StateMachine() {
	// TODO Auto-generated destructor stub
}

/**
 * If next state is valid, set state. Otherwise, set error state.
 * @param nextState The next state for the state machine
 */
void StateMachine::setState(SequencerState nextState) {
	switch (state) {
		case SequencerState::Stop:
			state = nextState;
			break;
		case SequencerState::Play:
			if (nextState == SequencerState::Stop) {
				state = nextState;
			}
			else{
				state = SequencerState::Error;
			}
			break;
		case SequencerState::Record:
			state = nextState;
			break;
		default:
			state = SequencerState::Error;
	}
}
