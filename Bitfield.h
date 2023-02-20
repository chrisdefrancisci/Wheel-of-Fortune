/*
 * bitfield.h
 *
 *  Created on: Feb 10, 2023
 *      Author: Chris
 *      https://stackoverflow.com/questions/30019454/operator-overload-in-bit-field-manipulation
 */

#ifndef BITFIELD_H_
#define BITFIELD_H_

#include <stdint.h>

template <uint8_t N> class Bitfield {
public:
	Bitfield();
	Bitfield(const Bitfield<N>& copy_from);
	virtual ~Bitfield();
	Bitfield<N> operator~();
	Bitfield<N> operator|(const Bitfield<N>&);
	Bitfield<N> operator&(const Bitfield<N>&);
	Bitfield<N> operator^(const Bitfield<N>&);
	Bitfield<N>& operator[](int i);
	const bool operator[](int i) const;
	Bitfield<N>& operator=(const Bitfield<N>& copy_from);
	Bitfield<N>& operator=(uint8_t* copy_buffer);
	Bitfield<N>& operator=(bool bit);
	operator bool();
private:
	uint8_t buffer[N] = { 0 };
	uint8_t buffer_idx;
	uint8_t mask;
};

#endif /* BITFIELD_H_ */
