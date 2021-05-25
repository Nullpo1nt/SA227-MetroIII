#ifndef TEST_H
#define TEST_H

#include "idealgas.h"

struct atmo_state {
    double temperature;  // K
    double pressure;  // Pa
    double density;  // kg/m^3
    double viscosity;  // μPa*s/m^2
};

struct engine_state {
    double rpm;
    bool bleedAirOn;
    bool engineDeiceOn;

    double designRpm;
};

struct engine_inlet {
    struct pvt out;
};

struct engine_compressor {
    struct pvt out;

    double compression_ratio;
};

struct engine_combuster {
    struct pvt out;

    double combustor_pressure_drop_percent;
};

struct engine_turbine {
    struct pvt out;
};

#endif