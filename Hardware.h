/*
 * Hardware.h
 *	Contains pin definitions
 *  Created on: Feb 7, 2023
 *      Author: Chris
 */

#ifndef HARDWARE_H_
#define HARDWARE_H_

#include "Arduino.h"

/** Debug LED setup register */
extern volatile uint8_t* pDebugLedSetup;
/** Debug LED port */
extern volatile uint8_t* pDebugLedPort;
/** Debug LED bit */
const uint8_t debugLed = 1 << 0;

/** Device (peripheral) reset setup register */
extern volatile uint8_t* pDevResetSetup;
/** Device (peripheral) reset port */
extern volatile uint8_t* pDevResetPort;
/** Device (peripheral) reset bit */
const uint8_t devReset = 1 << 1;

/** Gate 0 setup register */
extern volatile uint8_t* pGate0Setup;
/** Gate 0 port */
extern volatile uint8_t* pGate0Port;
/** Gate 0 bit */
const uint8_t gate0 = 1 << 5;

/** Gate 1 setup register */
extern volatile uint8_t* pGate1Setup;
/** Gate 1 port */
extern volatile uint8_t* pGate1Port;
/** Gate 1 bit */
const uint8_t gate1 = 1 << 6;

/** Gate 2 setup register */
extern volatile uint8_t* pGate2Setup;
/** Gate 2 port */
extern volatile uint8_t* pGate2Port;
/** Gate 2 bit */
const uint8_t gate2 = 1 << 7;

/** Gate 3 setup register */
extern volatile uint8_t* pGate3Setup;
/** Gate 3 port */
extern volatile uint8_t* pGate3Port;
/** Gate 3 bit */
const uint8_t gate3 = 1 << 0;

extern volatile uint8_t* pGatePortArray[4];
extern uint8_t gateArray[4];

// TODO(Chris): could also make above const pointers?

/**
 *
 */
typedef enum IOMap_def : uint8_t{
	BUTTON_STEP_00, /**< BUTTON_STEP_00 */
	BUTTON_STEP_01, /**< BUTTON_STEP_01 */
	BUTTON_STEP_02, /**< BUTTON_STEP_02 */
	BUTTON_STEP_03, /**< BUTTON_STEP_03 */
	BUTTON_STEP_04, /**< BUTTON_STEP_04 */
	BUTTON_STEP_05, /**< BUTTON_STEP_05 */
	BUTTON_STEP_06, /**< BUTTON_STEP_06 */
	BUTTON_STEP_07, /**< BUTTON_STEP_07 */
	BUTTON_STEP_08, /**< BUTTON_STEP_08 */
	BUTTON_STEP_09, /**< BUTTON_STEP_09 */
	BUTTON_STEP_10, /**< BUTTON_STEP_10 */
	BUTTON_STEP_11, /**< BUTTON_STEP_11 */
	BUTTON_OUT_1,   /**< BUTTON_OUT_1 */
	BUTTON_OUT_3, // Yes, this is the order, whoops.
	BUTTON_OUT_2,   /**< BUTTON_OUT_2 */
	BUTTON_OUT_4,   /**< BUTTON_OUT_4 */
	BUTTON_FUNC,    /**< BUTTON_FUNC */
	BUTTON_OCT_UP,  /**< BUTTON_OCT_UP */
	BUTTON_OCT_DOWN,/**< BUTTON_OCT_DOWN */
	BUTTON_PLAY     /**< BUTTON_PLAY */
} IOMap_t;

/**
 *
 */
typedef enum NoteMap_def : uint8_t{
	NOTE_i,   /**< NOTE_i */
	NOTE_i_s, /**< NOTE_i_s */
	NOTE_ii,  /**< NOTE_ii */
	NOTE_ii_s,/**< NOTE_ii_s */
	NOTE_iii, /**< NOTE_iii */
	NOTE_iv,  /**< NOTE_iv */
	NOTE_iv_s,/**< NOTE_iv_s */
	NOTE_v,   /**< NOTE_v */
	NOTE_v_s, /**< NOTE_v_s */
	NOTE_vi,  /**< NOTE_vi */
	NOTE_vi_s,/**< NOTE_vi_s */
	NOTE_vii  /**< NOTE_vii */
} NoteMap_t;

const char string_0[] PROGMEM = "step00";
const char string_1[] PROGMEM = "step01";
const char string_2[] PROGMEM = "step02";
const char string_3[] PROGMEM = "step03";
const char string_4[] PROGMEM = "step04";
const char string_5[] PROGMEM = "step05";
const char string_6[] PROGMEM = "step06";
const char string_7[] PROGMEM = "step07";
const char string_8[] PROGMEM = "step08";
const char string_9[] PROGMEM = "step09";
const char string_10[] PROGMEM = "step10";
const char string_11[] PROGMEM = "step11";
const char string_12[] PROGMEM = "out1";
const char string_13[] PROGMEM = "out3";
const char string_14[] PROGMEM = "out2"; // Yes, this is the order, whoops.
const char string_15[] PROGMEM = "out4";
const char string_16[] PROGMEM = "func";
const char string_17[] PROGMEM = "oct_up";
const char string_18[] PROGMEM = "oct_down";
const char string_19[] PROGMEM = "play";
const char *const string_table[] PROGMEM = {
		string_0, string_1, string_2, string_3, string_4, string_5,
		string_6, string_7, string_8, string_9, string_10, string_11,
		string_12, string_13, string_14, string_15, string_16, string_17,
		string_18, string_19
};

const uint8_t DAC_DRIVER_ADDRESS = B0001100;

void getProgmemString(int i, char* buffer);


#endif /* HARDWARE_H_ */
