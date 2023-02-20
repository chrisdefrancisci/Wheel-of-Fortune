/*
 * AD5695.h
 *
 *  Created on: Nov 22, 2022
 *      Author: Chris
 */

#ifndef AD5695_H_
#define AD5695_H_

#include "Arduino.h"
#include "CommStatus.h"
#include "Wire.h"

enum class DacAddr : uint8_t{
	/** DAC Addresses */
	DACA = 1<<0,
	DACB = 1<<1,
	DACC = 1<<2,
	DACD = 1<<3
};

DacAddr operator|= (const DacAddr& a, const DacAddr& b);
DacAddr operator| (const DacAddr& a, const DacAddr& b);
DacAddr operator+= (const DacAddr& a, const DacAddr& b);
DacAddr operator+ (const DacAddr& a, const DacAddr& b);


class AD5695 {
public:
	AD5695(uint8_t device_address);
	virtual ~AD5695();

	CommStatus begin();

	CommStatus writeVout(DacAddr dac_addr, uint16_t dac_data);
	CommStatus readVout(DacAddr dac_addr, uint16_t &dac_data);

private:
	const uint8_t _device_address;

};

#endif /* AD5695_H_ */
