#include "utils.h"
#include <cmath>

/*Translates unsigned 36-bit integer to signed 35-bit integer*/
int64_t uint36_sign(uint36 n){
    return (clearKthBit(n,35) * (getKthBit(n,35) ? -1 : 1));
}
    
/*Translates signed 2s-complement 35-bit integer to unsigned 36 bit integer*/
uint36 int36_unsign(int64_t n){
    return (abs(n) + ((n >= 0) ? 0 : lshft35));
}

/*Adds two uint36s, returns another uint36 with properly formatted sign*/
uint36 uint36_add(uint36 a, uint36 b){
    return int36_unsign(uint36_sign(a) + uint36_sign(b));
}