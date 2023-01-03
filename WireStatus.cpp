/*
 * WireStatus.cpp
 *
 *  Created on: Nov 22, 2022
 *      Author: Chris
 */

#include "WireStatus.h"

using namespace WireStatus;

/**
 * Prints errors in debug mode.
 * @param err: The error returned from the Wire library
 * @return: The status of the transmission
 */
ReturnStatus WireStatus::printWireStatus(uint8_t err){
	// TODO: Turn off prints when not debugging
	  switch (err)
	  {
	    case ReturnStatus::SUCCESS:
//			Serial.println("\tTransmitted without error.");
			break;
	    case ReturnStatus::DATA_LENGTH_ERROR:
			Serial.println("\tERROR: Data too long for buffer!!");
			break;
	    case ReturnStatus::NACK_ADDRESS_ERROR:
			Serial.println("\tERROR: Address not acknowledged!!");
			break;
	    case ReturnStatus::NACK_DATA_ERROR:
			Serial.println("\tERROR: Data not acknowledged!!");
			break;
	    case ReturnStatus::OTHER_ERROR:
			Serial.println("\tERROR: Other error!!");
			break;
	    default:
	    	Serial.println("\tERROR: Illegal return value!!");
	  }
	  return (ReturnStatus)(err);
}


