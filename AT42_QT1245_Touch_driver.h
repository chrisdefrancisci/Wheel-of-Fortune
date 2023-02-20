// Atmel AT42-QT1245 library
// Arduino library to interface with the capacitive touch sensor
// System block diagram
//  __________          ____________________
// | Arduino  |        | AT42AT1245 Sensor  |
// |          |        |                    |
// |       5V |--------| M_SYNC             |
// |      GND |--------| GND                |
// |        2 |--------| nCHANGE / S_SYNC   |
// |        4 |--------| nDRDY              |
// |        5 |--------| nSS                |
// |       11 |--------| MOSI               |
// |       12 |--------| MISO               |
// |       13 |--------| SCK                |
// |__________|        |                    |
//                     |                    |
//       __________    |                    |
//      | UTC78L05 |   |                    |
// 12V--|       5V |---| VCC                |
//      |      GND |---| GND                |
//      |__________|   |____________________| 

#ifndef ATMEL_AT42_QT1245
#define ATMEL_AT42_QT1245

#include "Arduino.h"
#include "SPI.h"
#include "CommStatus.h"
#include "Bitfield.h"

//#define DEVELOPMENT_BOARD
#define SEQUENCER_BOARD

// Constants
const uint8_t NCHANGE_PIN = 2; // for Arduino Uno, interrupt must be pin 2 or 3 (used as change interrupt so
// could be other pins)
#ifdef DEVELOPMENT_BOARD
const uint8_t NDRDY_PIN = 4; // AT42AT1245 data ready
const uint8_t CHIP_SELECT_PIN = 5; // AT42AT1245 chip select
const uint8_t ARDUINO_SELECT_PIN = 10; // Arduino chip select - needs to be set as output
#endif
#ifdef SEQUENCER_BOARD
const uint8_t NDRDY_PIN = A3; // AT42AT1245 data ready
const uint8_t CHIP_SELECT_PIN = 10; // AT42AT1245 chip select
const uint8_t ARDUINO_SELECT_PIN = 10; // Arduino chip select - needs to be set as output
#endif
const int poll_delay = 40; // microseconds
const uint32_t spi_clock = 400000L;
const SPISettings mySPISettings = SPISettings(spi_clock, MSBFIRST, SPI_MODE3); // max clock is 1.5MHz

// AT42AT1245 registers - memory map
const uint8_t QT1245_INIT_ADDR = 0; // Read this uint8_t when initializing
const uint8_t QT1245_COUNTER_100MS_ADDR = 2; // 100 ms counter (IEC/EN60730)
const uint8_t QT1245_COUNTER_SIGNAL_FAIL_ADDR = 3; // Signal fail counter (IEC/EN60730)
const uint8_t QT1245_COUNTER_MATRIX_SCAN_ADDR = 4; // Matrix Scan counter (IEC/EN60730)
const uint8_t QT1245_DEVICE_STATUS_ADDR = 5; // Device Status. Collection of bit flags
const uint8_t QT1245_DETECT_0_7_ADDR = 6; // Detect status for keys 0 to 7, one bit per key
const uint8_t QT1245_DETECT_8_15_ADDR = 7; // Detect status for keys 8 to 15, one bit per key
const uint8_t QT1245_DETECT_16_23_ADDR = 8; // Detect status for keys 16 to 23, one bit per key
const uint8_t QT1245_COMMAND_ADDR = 140; // write only location for commands
const uint8_t QT1245_SETUPS0_ADDR = 141; // Addresses 141 - 250 provide read/write access to the setups
    // with 141-164 being NTHR, PTHR, NDRIFT, BL for each key
const uint8_t QT1245_SETUPS1_ADDR = 165; // Addresses 165 - 188 provide read/write access to the setups
    // with 165 - 188 being WAKE, AKS, FDIL, NDIL for each key
const uint8_t QT1245_DWELL_RIB_THRM_FHM_ADDR = 244; // Changes for dwell, restart interrupted burst,
    // threshold multiplier (to NTHR and PTHR), and frequency hopping mode

// SPI commands
const uint8_t WRITE = 0b01111111;   //  write command
const uint8_t READ = 0b10000000;    // read command 0x80

// Command address commands
const uint8_t CALIBRATE_ALL = 0xFF; // recalibrates all keys and restarts operation
const uint8_t SETUPS_WR_EN = 0xFE; // enables setups / disengages setups write protection
// command addr is located conveniently immediately before setups s.t. write protection may be disengaged
// and setups written in single SPI communication sequence
const uint8_t LOW_LEVEL_CAL = 0xFD; // cal and offset across all keys and restarts operation. Takes up to 3s
const uint8_t FORCE_RESET = 0x18; // performs reset. after any reset, device automatically performs full key
// calibration of all keys
const uint8_t CALIBRATE_KEY = 0x00; // 0..23 requests calibration of that key

/** Number of bytes in the setup message data */
const uint8_t QT1245_SETUP_BYTES = 2;
/** Number of bytes used in a detect keys bitfield */
const uint8_t QT1245_DETECT_BYTES = 3;
/** Number of keys, corresponding to one bit each in a bitfield. */
const uint8_t QT1245_N_KEYS = 24;

/** Class variables */
constexpr uint8_t N_TOUCH_DRIVERS = 1;

// data structures
typedef union { 
    struct {
        // ? something like LSB order? 
        uint8_t NDIL : 3;
        uint8_t FDIL : 3;
        uint8_t AKS : 1;
        uint8_t WAKE : 1;
    };
    uint8_t data;
} keySetupsHigh_t;

typedef union { 
    struct {
        // ? something like LSB order?
        uint8_t NTHR : 3;
        uint8_t NDRIFT : 3;
        uint8_t BL : 2;
    };
    uint8_t data;
} keySetupsLow_t;

typedef struct {
    keySetupsHigh_t uint8_t1;
    keySetupsLow_t uint8_t0;
} keySetups_t;

class AT42_QT1245_Touch_driver {
public:
	AT42_QT1245_Touch_driver(uint8_t chip_select_pin, uint8_t nCHANGE_pin, uint8_t nDRDY_pin, uint8_t this_chip_select_pin);
	virtual ~AT42_QT1245_Touch_driver(void);
	CommStatus begin(void);
	CommStatus writeData(uint8_t address, uint8_t data);
	CommStatus writeData(uint8_t address, uint8_t nuint8_ts, uint8_t* pData);
	CommStatus readData(uint8_t address, uint8_t nuint8_ts, uint8_t* pData);
	CommStatus getKeySetups(keySetups_t &setups, int keyStart, int nKeys);
	CommStatus setKeySetups(keySetups_t setups, int keyStart, int nKeys);
	CommStatus getKeyStatus(Bitfield<3>& keyStatus);
	bool getNewTouch(void);
private:
	// Management of AT42_QT1245_Touch_driver across all instances
	static AT42_QT1245_Touch_driver* instances[N_TOUCH_DRIVERS];
	template <uint8_t DRIVER> static void handler() {
		instances[DRIVER]->setNewTouch();
	}

	void attach(void);
	static uint8_t driver_count;

	// Methods
	CommStatus AT42QT1245transfer(uint8_t dataIn, uint8_t* pDataOut);
	CommStatus AT42QT1245transfer(uint8_t dataIn);
	inline void setNewTouch(void) {
		new_touch = true;
		detachInterrupt(digitalPinToInterrupt(nCHANGE_pin)); // TODO: is this needed?
	}

	// Instance specific variables
	uint8_t driver_id = 0;
	volatile bool new_touch;
	uint8_t chip_select_pin;
	uint8_t nCHANGE_pin;
	uint8_t nDRDY_pin;
	uint8_t this_chip_select_pin;
};

#endif // ATMEL_AT42_QT1245
