#ifndef PID_H
#define PID_H

struct sc_pid_t
{
    double kp;
    double ki;
    //double kd;

    double error;
    double p;
    double i;
};

void pid_print(struct sc_pid_t *pid);
double pid_update(struct sc_pid_t *pid, double target, double actual, double deltaT);

#endif