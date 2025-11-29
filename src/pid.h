#pragma once
#include "q15.h"

typedef struct {
    q15 kp, ki, kd;
    q15 i_acc;
    q15 d_prev;
    q15 d_filt_state;
    q15 d_alpha;
    q15 i_limit;
    q15 u_min, u_max;
} pid_t;

q15 pid_step(pid_t* pid, q15 set, q15 meas);