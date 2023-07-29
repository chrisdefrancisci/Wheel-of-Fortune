/*
 * Note.h
 *
 *  Created on: Apr 14, 2023
 *      Author: Chris
 */

#ifndef NOTE_H_
#define NOTE_H_

#include "Arduino.h"

#include "AD5695.h"
#include "CommStatus.h"

class Note {
public:
	Note();
	virtual ~Note();
	CommStatus begin();
	CommStatus play(uint8_t sequencer, uint8_t note_idx);
	void stop(uint8_t sequencer);
private:
};

#endif /* NOTE_H_ */
