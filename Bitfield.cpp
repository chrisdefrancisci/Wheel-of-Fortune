/*
 * bitfield.cpp
 *
 *  Created on: Feb 10, 2023
 *      Author: Chris
 */

#include "Bitfield.h"

/**
 *
 * @tparam N
 */
template <uint8_t N> Bitfield<N>::Bitfield() {
	// TODO Auto-generated constructor stub
	mask = 0;
	buffer_idx = 0;
}


template <uint8_t N> Bitfield<N>::Bitfield(const Bitfield<N>& copy_from) {
	for (buffer_idx = 0; buffer_idx < N; buffer_idx++) {
		buffer[buffer_idx] = copy_from.buffer[buffer_idx];
	}
	buffer_idx = 0;
	mask = 0;
}

/**
 *
 * @tparam N
 */
template <uint8_t N> Bitfield<N>::~Bitfield() {
	// TODO Auto-generated destructor stub
	// Shouldn't need to delete buffer, because it wasn't allocated dynamically, right?
}

/**
 *
 * @tparam N
 * @return
 */
template <uint8_t N> Bitfield<N> Bitfield<N>::operator~() {
	Bitfield bitfield;
	for (uint8_t i = 0; i < N; i++) {
		bitfield.buffer[i] = ~buffer[i];
	}
	return bitfield;
}

/**
 *
 * @tparam N
 * @param b
 * @return
 */
template <uint8_t N> Bitfield<N> Bitfield<N>::operator|(const Bitfield<N>& b) {
	Bitfield bitfield;
	for (uint8_t i = 0; i < N; i++) {
		bitfield.buffer[i] = buffer[i] | b.buffer[i];
	}
	return bitfield;
}

/**
 *
 * @tparam N
 * @param b
 * @return
 */
template <uint8_t N> Bitfield<N> Bitfield<N>::operator&(const Bitfield<N>& b) {
	Bitfield bitfield;
	for (uint8_t i = 0; i < N; i++) {
		bitfield.buffer[i] = buffer[i] & b.buffer[i];
	}
	return bitfield;
}

/**
 *
 * @tparam N
 * @param b
 * @return
 */
template <uint8_t N> Bitfield<N> Bitfield<N>::operator^(const Bitfield<N>& b) {
	Bitfield bitfield;
	for (uint8_t i = 0; i < N; i++) {
		bitfield.buffer[i] = buffer[i] ^ b.buffer[i];
	}
	return bitfield;

}

/**
 *
 * @tparam N
 * @param i
 * @return
 */
template <uint8_t N> Bitfield<N>& Bitfield<N>::operator[](int i) {
	buffer_idx = 0;
	mask = 0;
	while (i >= 8) {
		buffer_idx++;
		i -= 8;
	}
	if (i < 0) {
		return *this; // TODO undefined
	}
	mask = 1 << i;
	return *this;
}


template <uint8_t N> Bitfield<N>& Bitfield<N>::operator=(const Bitfield<N>& copy_from) {
	for (buffer_idx = 0; buffer_idx < N; buffer_idx++) {
		buffer[buffer_idx] = copy_from.buffer[buffer_idx];
	}
	buffer_idx = 0;
	mask = 0;
	return *this;
}

/**
 * TODO this seems to swap the bit order on my mac???
 * @tparam N
 * @param copy_buffer
 * @return
 */
template <uint8_t N> Bitfield<N>& Bitfield<N>::operator=(uint8_t* copy_buffer) {
	for (buffer_idx = 0; buffer_idx < N; buffer_idx++) {
		buffer[buffer_idx] = copy_buffer[buffer_idx];
	}

	return *this;
}

/**
 *
 * @tparam N
 * @param bit
 * @return
 */
template <uint8_t N> Bitfield<N>& Bitfield<N>::operator=(bool bit) {
	if (bit) {
		buffer[buffer_idx] |= mask;
	}
	else {
		buffer[buffer_idx] &= ~mask;
	}

	return *this;
}

/**
 *
 * @tparam N
 */
template <uint8_t N>Bitfield<N>::operator bool() {
	return (buffer[buffer_idx] & mask) != 0;
}


template class Bitfield<1>;
template class Bitfield<2>;
template class Bitfield<3>;
template class Bitfield<4>;
