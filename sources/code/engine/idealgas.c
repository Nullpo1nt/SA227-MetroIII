#include "idealgas.h"


double adiabaticConstant(struct pvt *pvt) {
    return ADIABATIC_CONSTANT(pvt->pressure, pvt->volume, DOF_POLY);
}

double outTemperature(struct pvt *in, struct pvt *out) {
    return (out->pressure * out->volume) / ((in->pressure * in->volume) / in->temperature);
}