/*
 * WireStatus.cpp
 *
 *  Created on: Nov 22, 2022
 *      Author: Chris
 */

#include "CommStatus.h"


/**
 * Prints errors in debug mode.
 * @param err: The error returned from the Wire library
 * @return: The status of the transmission
 */
CommStatus printWireStatus(CommStatus err){
	// TODO: Turn off prints when not debugging
	  switch (err)
	  {
	    case CommStatus::Success:
//			Serial.println("\tTransmitted without error.");
			break;
	    case CommStatus::DataLengthError:
			Serial.println("\tERROR: Data too long for buffer!!");
			break;
	    case CommStatus::NackAddressError:
			Serial.println("\tERROR: Address not acknowledged!!");
			break;
	    case CommStatus::NackDataError:
			Serial.println("\tERROR: Data not acknowledged!!");
			break;
	    case CommStatus::OtherError:
			Serial.println("\tERROR: Other error!!");
			break;
	    default:
	    	Serial.println("\tERROR: Illegal return value!!");
	  }
	  return err;
}

/**
 * Overloaded or-equals operator.
 * @param a This object.
 * @param b The other object.
 * @return The bitwise "or" of the two objects.
 */
CommStatus operator|= (const CommStatus& a, const CommStatus& b) {
	return static_cast<CommStatus>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

/**
 * Overloaded and-equals operator.
 * @param a This object.
 * @param b The other object.
 * @return The bitwise "and" of the two objects.
 */
CommStatus operator&= (const CommStatus& a, const CommStatus& b) {
	return static_cast<CommStatus>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}
