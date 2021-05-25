#include <ncurses.h>

#include "pid.h"

void pid_print(struct sc_pid_t *pid)
{
    printw("p %3.1f*% f i %3.1f*% f e % f", pid->kp, (pid->p * pid->kp), pid->ki, (pid->i * pid->ki), pid->error);
}

double pid_update(struct sc_pid_t *pid, double target, double actual, double deltaT)
{
    double prev_error = pid->error;
    pid->error = target - actual;

    if ((prev_error <= 0 && pid->error >= 0) || (prev_error >= 0 && pid->error <= 0))
    {
        pid->i = 0;
    }

    pid->i += pid->error * deltaT;
    pid->p = pid->error;
    return (pid->kp * pid->error) + (pid->ki * pid->i); //+ (pid->kd * d);
}