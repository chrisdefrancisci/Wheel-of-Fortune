#include "Hardware.h"


/** Debug LED setup register */
volatile uint8_t* pDebugLedSetup = &DDRC;
/** Debug LED port */
volatile uint8_t* pDebugLedPort = &PORTC;
/** Debug LED bit */
//const uint8_t debugLed = 1 << 0;

/** Device (peripheral) reset setup register */
volatile uint8_t* pDevResetSetup = &DDRB;
/** Device (peripheral) reset port */
volatile uint8_t* pDevResetPort = &PORTB;
/** Device (peripheral) reset bit */
//const uint8_t devReset = 1 << 1;

/** Gate 0 setup register */
volatile uint8_t* pGate0Setup = &DDRD;
/** Gate 0 port */
volatile uint8_t* pGate0Port = &PORTD;
/** Gate 0 bit */
//const uint8_t gate0 = 1 << 5;

/** Gate 1 setup register */
volatile uint8_t* pGate1Setup = &DDRD;
/** Gate 1 port */
volatile uint8_t* pGate1Port = &PORTD;
/** Gate 1 bit */
//const uint8_t gate1 = 1 << 6;

/** Gate 2 setup register */
volatile uint8_t* pGate2Setup = &DDRD;
/** Gate 2 port */
volatile uint8_t* pGate2Port = &PORTD;
/** Gate 2 bit */
//const uint8_t gate2 = 1 << 7;

/** Gate 3 setup register */
volatile uint8_t* pGate3Setup = &DDRB;
/** Gate 3 port */
volatile uint8_t* pGate3Port = &PORTB;
/** Gate 3 bit */
//const uint8_t gate3 = 1 << 0;

volatile uint8_t* pGatePortArray[] = {pGate0Setup, pGate1Setup, pGate2Setup, pGate3Setup};
uint8_t gateArray[] = {gate0, gate1, gate2, gate3};

/**
 * Helper function to return a string from the table.
 * @param[in] i Index of the string to grab.
 * @param[out] buffer Buffer to place string into.
 */
void getProgmemString(int i, char* buffer) {
	if (i > 19 || i < 0) {
		strcpy(buffer, "None.");
	}
	strcpy_P(buffer, (char *)pgm_read_word(&(string_table[i])));
}
