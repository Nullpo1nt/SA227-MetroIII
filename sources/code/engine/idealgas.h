#ifndef _IDEALGAS_H_
#define _IDEALGAS_H_

#include <math.h>

#define DOF_POLY (7.0 / 5.0)
#define DOF_POLY_INV (5.0 / 7.0)

#define ADIABATIC_CONSTANT(p, v, dof)  (p * pow(v, dof))
// #define ADIABATIC_WORK(k, v_in, v_out, fod) 

struct pvt {
    double pressure;
    double volume;
    double temperature;
};


double adiabaticConstant(struct pvt *pvt);
double outTemperature(struct pvt *in, struct pvt *out);

#endif
