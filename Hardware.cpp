#include "Hardware.h"

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
