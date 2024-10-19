#ifndef UTILS_H
#define UTILS_H

#include <cstdint>

using uint36 = uint64_t;
using uint38 = uint64_t;
using uint18 = uint32_t;
using uint15 = uint16_t;

#define lshft35 ((uint36)1 << 35) //Equivalent to 2^35
#define lshft37 ((uint38)1 << 37) //Equivalent to 2^37

/*Returns the kth bit of integer n. Least significant bit is at index 0*/
inline uint64_t getKthBit(uint64_t n, uint64_t k){
    return (n & ( (uint64_t)1 << k )) >> k;
}

/*Sets the kth bit of n to 1. Least significant bit is at index 0*/
inline uint64_t setKthBit(uint64_t n, int k){
    return n | (uint64_t)1 << k;
}

/*Clears the kth bit of n to 0. Least significant bit is at index 0*/
inline uint64_t clearKthBit(uint64_t n, int k){
    return n & ~ ((uint64_t)1 << k);
}

/*Toggles the kth bit of n. Least significant bit is at index 0*/
inline uint64_t toggleKthBit(uint64_t n, int k){
    return n ^ ((uint64_t)1 << k);
}

/*Translates unsigned 36-bit integer to signed 35-bit integer*/
extern int64_t uint36_sign(uint36 n);
/*Translates signed 2s-complement 35-bit integer to unsigned 36 bit integer*/
extern uint36 int36_unsign(int64_t n);
/*Adds two uint36s, returns another uint36 with properly formatted sign*/
extern uint36 uint36_add(uint36 a, uint36 b);

#endif

