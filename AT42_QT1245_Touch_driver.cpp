#include "AT42_QT1245_Touch_driver.h"

/**
 * Constructor. Adds static reference to this object.
 * @param chip_select_pin Chip Select pin for Peripheral device.
 * @param nCHANGE_pin ~Change pin, used to alert Controller that a new touch has occurred.
 * @param nDRDY_pin ~Data Ready pin, used to alert Controller that SPI communication can proceed.
 * 		This pin must have a Pin Change interrupt associated with it.
 * @param this_chip_select_pin Chip Select pin for controller device. Set to output so controller
 * 		does not accidentaly become peripheral.
 */
AT42_QT1245_Touch_driver::AT42_QT1245_Touch_driver(uint8_t chip_select_pin, uint8_t nCHANGE_pin, uint8_t nDRDY_pin, uint8_t this_chip_select_pin) :
	chip_select_pin(chip_select_pin),
	nCHANGE_pin(nCHANGE_pin),
	nDRDY_pin(nDRDY_pin),
	this_chip_select_pin(this_chip_select_pin)
{
	// TODO assert that nDRDY has pin change interrupt
	driver_id = driver_count;
	driver_count++;
	instances[driver_id] = this;
}

/**
 * Destructor. Removes static reference to this object.
 */
AT42_QT1245_Touch_driver::~AT42_QT1245_Touch_driver(void)
{
	instances[driver_id] = NULL;
	driver_count--;
}

/**
 * Initialization function
 * The host should repeatedly wait for a period not less than the QT1245 communications time-out
 * (30 ms ±5 ms). The host should then read location 0, followed by the CRC uint8_ts, until the
 * correct response is received back from location 0 and validated by the CRC. Location 0 should
 * read as 19 hex (25 decimal). The host can then resume normal run mode communications.
 * @return Communication status.
 */
CommStatus AT42_QT1245_Touch_driver::begin(void)
{
  const uint8_t init_addr = 0x00;
  const uint8_t init_n = 1;
  const uint8_t init_const = 0x19;
  const int startup_time = 95;
  uint8_t init_return = 0x00;

  int attempts = 0;
  const int max_attempts = 10;
  SPI.begin();
  pinMode(chip_select_pin, OUTPUT);
  pinMode(this_chip_select_pin, OUTPUT);
  pinMode(nCHANGE_pin, INPUT); // TODO: !!! MUST ATTACH INTERRUPT FOR THIS TO WORK !!!
  pinMode(nDRDY_pin, INPUT);
  delay(startup_time); // a max of 95 ms is required from reset before the edevice is ready to communicate


  while(init_return != init_const)
  {
    readData(init_addr, init_n, &init_return);
    Serial.flush();
    delay(40);
    attempts++;
    if (attempts > max_attempts)
    {
      Serial.println("Max attempts exceeded. Unable to initialize ATQT1245");
      Serial.flush();
      return CommStatus::OtherError;
    }
  }

  // Clear nCHANGE by reading from register 6 (but 7, 8, should also be read)
  KeyStatus_t dummy = { 0 };
  pKeyStatus_t pDummy;
  pDummy.pKeyStatus = &dummy;
  getKeyStatus(pDummy);

  Serial.println("ATQT1245 Initialized.");
  Serial.flush();

  return CommStatus::Success;
}

/**
 * Exchanges one byte via SPI with nDRDY checking.
 * @param[in] dataIn Data to transfer to Peripheral from Controller.
 * @param[out] pDataOut Data to sent from Peripheral to Controller.
 * @return Communication status.
 */
CommStatus AT42_QT1245_Touch_driver::AT42QT1245transfer(uint8_t dataIn, uint8_t* pDataOut) {
  const int max_attempts = 450; // reads are guaranteed in <1ms, writes <4.5ms
  int attempts = 0;
  digitalWrite(chip_select_pin, LOW);
  while(digitalRead(nDRDY_pin) == LOW)
  {
    attempts++;
    if (attempts > max_attempts)
    {
      return CommStatus::OtherError;
    }
    delayMicroseconds(10);
  }
  // take the chip select low to select the device:
  *pDataOut = SPI.transfer(dataIn); // Send data uint8_t
  digitalWrite(chip_select_pin, HIGH);
  delayMicroseconds(40); // delay between each transfer as specified on pg 23 of datasheet
  return CommStatus::Success;
}

/**
 * Wrapper function if Peripheral data is not expected.
 * @param[in] dataIn
 * @return Communication status.
 */
CommStatus AT42_QT1245_Touch_driver::AT42QT1245transfer(uint8_t dataIn) {
  uint8_t throwaway = 0x00;
  return AT42QT1245transfer(dataIn, &throwaway);
}

/**
 * @brief Writes a buffer of data to a specified address of the Peripheral.
 * The host initiates a write sequence by sending the internal memory address it wishes
 * to write followed by n, the number of QT1245 addresses it wishes to write to. The
 * host then sends one or more data uint8_ts.
 * @param[in] address Peripheral memory address to write.
 * @param[in] nuint8_ts Number of bytes.
 * @param[in] pData Pointer to uint8_t data buffer.
 * @return Communication status.
 */
CommStatus AT42_QT1245_Touch_driver::writeData(uint8_t address, uint8_t nuint8_ts, uint8_t* pData) {
  CommStatus success = CommStatus::Success; // error return value, 0=failure, 1 = success
  Serial.print("Writing "); Serial.print(nuint8_ts); Serial.print(" to register ");
  Serial.println(address);

  if(nuint8_ts > 111)
  {
    nuint8_ts = 111;
    // Serial.println("Max number of uint8_ts is 111. Truncating.");
  }
  
  // Clock normally high (CPOL=1) and data sampled on rising edge (CPHA=1)->SPI_MODE3
  SPI.beginTransaction(mySPISettings);
  success |= AT42QT1245transfer(address); //Send register location
  
  // uint8_t 2 is split into two fields. The MSB indicates a read or write operation and should
  // be set to 0 to perform a write. Bits 6 – 0 define the number of data uint8_ts the host wishes
  // to write to the device.
  success |= AT42QT1245transfer(nuint8_ts & WRITE); // setting write bit unnecessary due to above truncation
  
  while(nuint8_ts > 0)
  {
    success &= AT42QT1245transfer(*pData);  //Send value to record into register
    pData++; // increment pointer to next data value
    nuint8_ts--; // decrease number of uint8_ts remaining to send
  }

  SPI.endTransaction();

  return success;
}

/**
 * Write wrapper for single byte.
 * @param[in] address Peripheral memory address to write.
 * @param[in] data Byte to write.
 * @return Communication status.
 */
CommStatus AT42_QT1245_Touch_driver::writeData(uint8_t address, uint8_t data)
{
	return writeData(address, 1, &data);
}

/**
 * @brief Read from registers.
 * The host initiates a write sequence by sending the internal
 * memory address it wishes to write followed by n, the number of
 * QT1245 addresses it wishes to write to. The host then sends NULL
 * uint8_ts to recover the n data uint8_ts from the QT1245. If the host specifies n to
 * be greater than 1, the data uint8_ts are read from consecutive memory addresses.
 * The device automatically increments the target memory address after reading
 * each location. When all n data uint8_ts have been returned, the device returns a
 * 16-bit CRC, LSB first.
 * @param[in] address Peripheral memory address to write.
 * @param[in] nuint8_ts Number of bytes to read.
 * @param[out] pData Buffer to place the data.
 * @return Communication status.
 */
CommStatus AT42_QT1245_Touch_driver::readData(uint8_t address, uint8_t nuint8_ts, uint8_t* pData) {
  uint8_t CRC_LSB;
  uint8_t CRC_MSB;
  uint8_t nuint8_ts_copy = nuint8_ts;
  uint8_t *pData_copy = pData;
  CommStatus success = CommStatus::Success; // error return value, 0=failure, 1 = success
  Serial.print("Reading "); Serial.print(nuint8_ts); Serial.print(" uint8_t(s) from register 0x0");
  Serial.print(address, HEX); Serial.print(":\t"); Serial.flush();
  
  // take the chip select low to select the device:
  SPI.beginTransaction(mySPISettings);
  //digitalWrite(chip_select_pin, LOW);
  // send the device the register you want to read:
  success |= AT42QT1245transfer(address);

  success |= AT42QT1245transfer(nuint8_ts | READ); // send read command
  
  while(nuint8_ts > 0)
  {
    success |= AT42QT1245transfer(0x00, pData);  //Send value to record into register

    pData++; // increment pointer to next data value
    nuint8_ts--; // decrease number of uint8_ts remaining to send
  }
  
  // read the two-uint8_t CRC
  success |= AT42QT1245transfer(0x00, &CRC_LSB);
  success |= AT42QT1245transfer(0x00, &CRC_MSB);


  // take the chip select high to de-select:
  //digitalWrite(chip_select_pin, HIGH);
  SPI.endTransaction();
  
  if (success == CommStatus::Success)
  {
    while(nuint8_ts_copy > 0)
    {
      Serial.print("0x"); Serial.print(*pData_copy, HEX); Serial.print("\t"); Serial.flush();
      pData_copy++;
      nuint8_ts_copy--;
    }
  }
  else
  {
    Serial.print("fail");
  }
  Serial.println();
  // todo: check the CRC
  // Serial.print("CRC = ");
  // Serial.println((CRC_MSB<<8) | CRC_LSB, HEX);
  Serial.flush();
  
  // return the result:
  return success;
}

/**
 * Get the setup information for keys/buttons.
 * @param setups[out] Buffer to place the key setups.
 * @param keyStart[in] First key to examine.
 * @param nKeys[in] Number of key setups to place in the buffer.
 * @return Communication status.
 */
CommStatus AT42_QT1245_Touch_driver::getKeySetups(keySetups_t* setups, int keyStart, int nKeys) {
	CommStatus success = CommStatus::Success;
	for (int i = 0; i < nKeys; i++){
		success |= readData(QT1245_SETUPS0_ADDR + keyStart + i, 1,
			&(setups->uint8_t0.data));
	}
	for (int i = 0; i < nKeys; i++){
		success |= readData(QT1245_SETUPS1_ADDR + keyStart + i, 1,
			&(setups->uint8_t1.data));
	}
	return success;
}

/**
 * Set the setup information for keys/buttons.
 * @param setups[in] Buffer containing key setups.
 * @param keyStart[in] First key to set.
 * @param nKeys[in] Number of keys to set.
 * @return Communication status.
 */
CommStatus AT42_QT1245_Touch_driver::setKeySetups(keySetups_t* setups, int keyStart, int nKeys) {
	CommStatus success = CommStatus::Success;
	// setups must be write-enabled by writing 0xFE to the Command Address immediately before writing the
	// setups themselves. Command address is conveniently before setups address.
	// writing new setups, changes take place immediately, values are recorded into EEPROM
	writeData(QT1245_COMMAND_ADDR, SETUPS_WR_EN);

	for (int i = 0; i < nKeys; i++){
		success |= writeData(QT1245_SETUPS0_ADDR + keyStart + i,
			(uint8_t)(setups->uint8_t0.data));
	}
	for (int i = 0; i < nKeys; i++){
		success |= writeData(QT1245_SETUPS1_ADDR + keyStart + i,
			(uint8_t)(setups->uint8_t1.data));
	}

	// device should be restarted - includes auto calibration
	Serial.println("Restarting...");
	writeData(QT1245_COMMAND_ADDR, FORCE_RESET);
	delay(3000);

	// attempt to initialize device
	while(begin() != CommStatus::Success)
	{
	Serial.println("Unable to contact sensor. Waiting 2 seconds.");
	delay(2000);
	}

	return success;
}

/**
 * Transfer key/button press data from Peripheral to Controller.
 * @param pKeyStatus Buffer to place the key data.
 * @return Communication status.
 */
CommStatus AT42_QT1245_Touch_driver::getKeyStatus(pKeyStatus_t pKeyStatus)
{
  const int keyStatusSize = 3;
  const int keyStatusAddr = 0x06;
  return readData(keyStatusAddr, keyStatusSize, pKeyStatus.pKeyStatusUint8_t);
}


/** Definitions and initializers for static members */
AT42_QT1245_Touch_driver* AT42_QT1245_Touch_driver::instances[N_TOUCH_DRIVERS] = {};
uint8_t AT42_QT1245_Touch_driver::driver_count = 0;
