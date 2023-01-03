/*
 * WireStatus.h
 *
 *  Created on: Nov 22, 2022
 *      Author: Chris
 */

#ifndef WIRESTATUS_H_
#define WIRESTATUS_H_

#include "Arduino.h"

// Wire library constants
// End transmission return status
// TODO: convert to enum class with a static_cast function to uint8_t
namespace WireStatus{
enum ReturnStatus : uint8_t {
SUCCESS = 0,
DATA_LENGTH_ERROR = 1, // data too long to fit in buffer
NACK_ADDRESS_ERROR = 2, // received NACK on transmit of address
NACK_DATA_ERROR = 3, // received NACK on transmit of data
OTHER_ERROR = 4
};

ReturnStatus printWireStatus(uint8_t err);

}
#endif /* WIRESTATUS_H_ */
