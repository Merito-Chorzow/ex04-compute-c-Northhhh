#pragma once
#include "q15.h"

// y[k+1] = y[k] + alpha*( -y[k] + u[k] )
static inline q15 plant_step(q15 y, q15 u, q15 alpha){
    q15 dif = (q15)((int32_t)u - (int32_t)y);
    q15 delta = mul_q15(alpha, dif);
    int32_t y32 = (int32_t)y + (int32_t)delta;
    return sat16(y32);
}
