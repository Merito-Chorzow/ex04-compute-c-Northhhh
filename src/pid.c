#include "pid.h"

q15 pid_step(pid_t* pid, q15 set, q15 meas){
    // błąd
    q15 e = set - meas;

    // --- P ---
    int32_t p_raw = (int32_t)pid->kp * (int32_t)e; // Q15*Q15 => Q30
    q15 p_term = (q15)(p_raw >> 15);

    // --- I (clamping anti-windup) ---
    int32_t i_new = (int32_t)pid->i_acc + (((int32_t)pid->ki * (int32_t)e) >> 15);
    if (i_new >  pid->i_limit) i_new =  pid->i_limit;
    if (i_new < -pid->i_limit) i_new = -pid->i_limit;
    pid->i_acc = (q15)i_new;

    // --- D z filtrem ---
    q15 de = e - pid->d_prev;
    // y = alpha*y_prev + (1-alpha)*de
    q15 one_minus_alpha = (q15)(32767 - pid->d_alpha);
    q15 d_filt = (q15)( ((int32_t)pid->d_alpha * (int32_t)pid->d_prev >> 15)
                      + ((int32_t)one_minus_alpha * (int32_t)de >> 15) );
    pid->d_prev = e;

    // --- suma i saturacja ---
    int32_t sum = (int32_t)p_term + (int32_t)pid->i_acc + (int32_t)d_filt;
    if (sum > pid->u_max) sum = pid->u_max;
    if (sum < pid->u_min) sum = pid->u_min;

    return (q15)sum;
}
