/*
 * WireStatus.h
 *
 *  Created on: Nov 22, 2022
 *      Author: Chris
 */

#ifndef COMM_STATUS_H_
#define COMM_STATUS_H_

#include "Arduino.h"

// Wire library constants
// End transmission return status
// TODO: convert to enum class with a static_cast function to uint8_t
enum class CommStatus : uint8_t {
	Success = 0,
	DataLengthError = 1, // data too long to fit in buffer
	NackAddressError = 2, // received NACK on transmit of address
	NackDataError = 4, // received NACK on transmit of data
	OtherError = 8
};

CommStatus printWireStatus(CommStatus err);
inline CommStatus printWireStatus(uint8_t err){ return printWireStatus(CommStatus(err)); };

CommStatus operator|= (const CommStatus& a, const CommStatus& b);

CommStatus operator&= (const CommStatus& a, const CommStatus& b);

#endif /* COMM_STATUS_H_ */
