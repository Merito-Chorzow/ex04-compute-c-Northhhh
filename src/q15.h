#pragma once
#include <stdint.h>

typedef int16_t q15;          // 1.15 fixed-point
static inline q15 f2q15(float x){
    if (x > 0.999969f) x = 0.999969f;
    if (x < -1.0f)     x = -1.0f;
    return (q15)(x * 32768.0f);
}
static inline float q15tof(q15 q){
    return (float)q / 32768.0f;
}
static inline q15 sat16(int32_t x){
    if (x >  32767) return  32767;
    if (x < -32768) return -32768;
    return (q15)x;
}
static inline q15 mul_q15(q15 a, q15 b){
    return (q15)(((int32_t)a * (int32_t)b) >> 15);
}
