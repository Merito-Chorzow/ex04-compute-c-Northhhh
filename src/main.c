#include <stdio.h>
#include "q15.h"
#include "pid.h"
#include "plant.h"

int main(void){
    pid_t pid = {
        .kp = f2q15(0.6f),
        .ki = f2q15(0.05f),
        .kd = f2q15(0.01f),
        .i_acc = 0,
        .d_prev = 0,
        .d_filt_state = 0,
        .d_alpha = f2q15(0.85f),
        .i_limit = f2q15(0.5f),
        .u_min = f2q15(-1.0f),
        .u_max = f2q15( 1.0f),
    };

    q15 y = f2q15(0.0f);
    q15 set = f2q15(0.5f);
    q15 alpha = f2q15(0.05f);

    for(int k=0;k<1000;k++){
        q15 u = pid_step(&pid, set, y);
        y = plant_step(y, u, alpha);

        if(k % 50 == 0){
            printf("k=%4d set=%+.3f y=%+.3f u=%+.3f\n",
                k, q15tof(set), q15tof(y), q15tof(u));
        }
    }
    return 0;
}
