#include "pid.h"

q15 pid_step(pid_t* pid, q15 set, q15 meas){
    q15 e = set - meas;

    q15 p_term = mul_q15(pid->kp, e);

    int32_t i_temp = (int32_t)pid->i_acc + (int32_t)mul_q15(pid->ki, e);
    if (i_temp >  pid->i_limit) i_temp =  pid->i_limit;
    if (i_temp < -pid->i_limit) i_temp = -pid->i_limit;
    pid->i_acc = (q15)i_temp;

    q15 de = e - pid->d_prev;
    q15 d_raw = mul_q15(pid->kd, de);

    q15 one_minus_alpha = (q15)(32767 - pid->d_alpha);
    q15 d_val = mul_q15(pid->d_alpha, pid->d_filt_state) 
              + mul_q15(one_minus_alpha, d_raw);
    
    pid->d_filt_state = d_val;
    pid->d_prev = e;

    int32_t sum = (int32_t)p_term + (int32_t)pid->i_acc + (int32_t)d_val;

    if (sum > pid->u_max) sum = pid->u_max;
    if (sum < pid->u_min) sum = pid->u_min;

    return (q15)sum;
}