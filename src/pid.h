#pragma once
#include "q15.h"

typedef struct {
    q15 kp, ki, kd;     // wzmocnienia
    q15 i_acc;          // akumulator całki
    q15 d_prev;         // poprzedni błąd (dla D)
    q15 d_alpha;        // filtr D (0..1 w q15)
    q15 i_limit;        // ograniczenie całki (|i_acc| <= i_limit)
    q15 u_min, u_max;   // saturacja wyjścia
} pid_t;

// Jedno wywołanie kroku regulatora
q15 pid_step(pid_t* pid, q15 set, q15 meas);
