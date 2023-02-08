/*
 * AD5695.cpp
 *
 *  Created on: Nov 22, 2022
 *      Author: Chris
 */

#include "AD5695.h"


/** Command Bytes */
const uint8_t NOP = 0x00; /** No operation */
const uint8_t WRITE = 0x01<<4; /** Write to DAC register(s). When ~LDAC is low, register is "transparent"  */
const uint8_t WRITE_UPDATE = 0x03<<4; /** Write to DAC registers and update outputs directly */

AD5695::AD5695(uint8_t device_address) :
		_device_address(device_address)
{
	// TODO Auto-generated constructor stub

}

AD5695::~AD5695() {
	// TODO Auto-generated destructor stub
}

CommStatus AD5695::begin() {
	Wire.begin();
	return CommStatus::Success;
}


CommStatus AD5695::writeVout(DacAddr dac_addr, uint16_t dac_data) {
	CommStatus status =  CommStatus::Success;
	uint8_t safe_addr = dac_addr & 0x0F; // Make sure MSBs are 0
	dac_data <<= 2; // Put the 14 valid bits in the MSBs TODO make extensible to all DACs in series
	Wire.beginTransmission(_device_address); // Device address
	Wire.write(WRITE_UPDATE | safe_addr); // Command (bytes 7:4) | DAC address (bytes 3:0)
	Wire.write(uint8_t(dac_data >> 8)); // DAC data 8 MSBs
	Wire.write(uint8_t(dac_data)); // DAC data 6 LSBs
	status = (CommStatus)(Wire.endTransmission());
	return status;
}

//TODO:Utilize ~LDAC register to create update, write+update functions


CommStatus AD5695::readVout(DacAddr dac_addr, uint16_t& dac_data) {
	CommStatus status =  CommStatus::Success;
	uint16_t data;
	// TODO: safe address should only be 1,2,4,8 - not some combination thereof
	uint8_t safe_addr = dac_addr & 0x0F; // Make sure MSBs are 0
	dac_data = 0; // Clear
	Wire.beginTransmission(_device_address); // Device address
	Wire.write(NOP | safe_addr); // Command (bytes 7:4) | DAC address (bytes 3:0)
	status = (CommStatus)(Wire.endTransmission(false));
	Wire.requestFrom(_device_address, (uint8_t)2, (uint8_t)true);
	data = Wire.read();
	dac_data = data << 8;
	data = Wire.read();
	dac_data |= data;
	return status;
}
