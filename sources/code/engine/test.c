#include <ncurses.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

#include "test.h"
#include "pid.h"

#define ATMO_STD_TEMPERATURE 288.15
#define ATMO_STD_PRESSURE 101325.0
#define ATMO_STD_DENSITY 1.229
#define ATMO_STD_VISCOUSITY 17.3

#define C_TO_K(x) ((x) + 273.15)
#define K_TO_C(x) ((x) - 273.15)

#define W_TO_HP(x) ((x) * 0.0013)
#define HP_TO_TORQUE(x, rpm) (((x) * 5252.0) / (rpm))

#define STD_ATM_P 100000.0

int running = 1;

struct sc_pid_t pg_out_pi = {.kp = 9.0, .ki = 0.5, .i = 0};
struct sc_pid_t fc_underspeed_pi = {.kp = 20.0, .ki = 5.0, .i = 0};

double e_external_load = 1.0; // N

double e_ff = 1.0;
double e_rpm = 41730.0;

double deiceOn = 0.0;
double bleedOn = 0.0;

double awiOn = 0.0;

// STD day
struct atmo_state atmosphere = { .temperature = C_TO_K(15.0), .pressure = 101325.0, .density = 1.229, .viscosity=17.3 };

void tick() {
    const int timeMS = 25;
    static struct timespec ts;
    ts.tv_sec = timeMS / 1000;
    ts.tv_nsec = (timeMS % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

void printPVT(const char *label, struct pvt *p) {
    printw("%s %6.1f C, %8.0f Pa, %4.2f m^3\n", label, K_TO_C(p->temperature), p->pressure, p->volume);
}

void engineInlet(struct engine_inlet* inlet, struct atmo_state *in, double rpm) {
    // PV = nRT
    // 3.49266 kg/s / STD_DEN = 2.84 m^3 / s
    // P (V) = n R T
    inlet->out.temperature = in->temperature;
    inlet->out.pressure = in->pressure;
    inlet->out.volume = ((3.49266 / ATMO_STD_DENSITY) * (in->density / ATMO_STD_DENSITY)) * (rpm / 41730.0);
}

void engineCompress(struct engine_compressor* c, struct pvt *in, double rpm) {
    double c_k_in = ADIABATIC_CONSTANT(in->pressure, in->volume, DOF_POLY);

    c->out.pressure = in->pressure * (1.0 + ((e_rpm / 41730.0) * 9.55)); // 10.55 compression...
    c->out.volume = pow(c_k_in / c->out.pressure, DOF_POLY_INV);
    c->out.temperature =  outTemperature(in, &c->out);
}

void engineBleed(struct pvt *b, struct pvt *in, double bleedOn, double deiceOn) {
    double c_k_in = ADIABATIC_CONSTANT(in->pressure, in->volume, DOF_POLY);

    if (bleedOn > 0.01) {
        double d = (1.0 - bleedOn - deiceOn);
        b->volume = in->volume * d;
        b->pressure = in->pressure * d;
    } else {
        b->volume = in->volume;
        b->pressure = in->pressure;
    }
    //b->pressure = c_k_in / pow(b->volume, DOF_POLY);
    b->temperature =  in->temperature;//(b->pressure * b->volume) / ((in->pressure * in->volume) / in->temperature);
}

void engineCombustion(struct engine_combuster *state, struct pvt *in, double ff) {
    // TODO: Implement better combustion physics
    // JET-A 35.3 MJ/L
    //       43.02 MJ/kg
    // Kerosene burns at 2366.15 K
    state->out.temperature = in->temperature + ((ff * 2366.15) * (101325.0 / (in->pressure*in->volume)));// (0.53 / in->volume));
    state->out.pressure = in->pressure;// * state->combustor_pressure_drop_percent;
    state->out.volume = (((in->pressure * in->volume) / in->temperature) * state->out.temperature) / state->out.pressure;
}

void engineTurbine(struct engine_turbine *state, struct pvt *in, struct atmo_state *a) {
    double t_k = ADIABATIC_CONSTANT(in->pressure, in->volume, DOF_POLY);

    // Pressure drop percentage...
    // state->out.pressure = in->pressure / 5.0;
    // state->out.volume = pow(t_k / state->out.pressure, 1.0 / DOF_POLY);

    // Volume increase...
    // state->out.volume = in->volume * 6.5;
    // state->out.pressure = t_k / pow(state->out.volume, DOF_POLY);

    // Drop to atmo pressure...
    state->out.pressure = a->pressure;
    state->out.volume = pow(t_k / state->out.pressure, DOF_POLY_INV);

    state->out.temperature = (state->out.pressure * state->out.volume) / ((in->pressure * in->volume) / in->temperature);

    // double t_W = (t_k * (pow(t_v_out, 1.0 / DOF_POLY) - pow(c_v_out, fod))) / fod;
}

void engineExhaust(struct pvt *exhaust, struct pvt *in, struct atmo_state *a) {
    double t_k = ADIABATIC_CONSTANT(in->pressure, in->volume, DOF_POLY);
    exhaust->pressure = a->pressure;
    exhaust->volume = pow(t_k / exhaust->pressure, 1.0 / DOF_POLY);
    exhaust->temperature =  (exhaust->pressure * exhaust->volume) / ((in->pressure * in->volume) / in->temperature);
}

void *engineBackground(void *vargs) // air_model_t *air, engine_model_t *engine, engine_state_t *state)
{
    long t = 0;
    const double timeMSd = 0.025;

    struct engine_inlet stage_inlet = {};
    struct engine_compressor stage_compressor = { .compression_ratio = 10.55 };
    struct pvt stage_bleed = {};
    struct engine_combuster stage_combuster = { .combustor_pressure_drop_percent = 0.985 };
    struct engine_turbine stage_turbine = {};
    // struct pvt exhaust = {};

    double v = 3.491266 /* kg/s */;
    double e_inlet_volume = 100.0;

    while (running) {
        const double dof = 7.0 / 5.0;
        const double fod = 1.0 - dof;

        engineInlet(&stage_inlet, &atmosphere, e_rpm);
        engineCompress(&stage_compressor, &stage_inlet.out, e_rpm);
        double c_W = (adiabaticConstant(&stage_compressor.out) * (pow(stage_compressor.out.volume, fod) - pow(stage_inlet.out.volume, fod))) / fod;
        engineBleed(&stage_bleed, &stage_compressor.out, bleedOn, deiceOn);
        // stage_compressor.out.volume = stage_compressor.out.volume * 0.974;
        engineCombustion(&stage_combuster, &stage_bleed, e_ff);
        engineTurbine(&stage_turbine, &stage_combuster.out, &atmosphere);
        double t_k = adiabaticConstant(&stage_combuster.out); 
        double t_W = (t_k * (pow(stage_turbine.out.volume, fod) - pow(stage_combuster.out.volume, fod))) / fod;
        t_W *= 0.8;

        if (t % 4 == 0)
        {
            clear();
            printw("Controls\n1: %0.2f FF    2: %0.0f RPM     3: %0.3f Air Density     4: %0.0f C (Atmo)\n", e_ff, e_rpm, atmosphere.density, K_TO_C(atmosphere.temperature));
            printw("a: Bleed %i   s: Deice %i       d: AWI %i\n", (bleedOn > 0), (deiceOn>0), (awiOn>0));
            printw("\tAir: %0.1f K, %0.0f Pa, %0.2f kg/m^3\n\n", atmosphere.temperature, atmosphere.pressure, atmosphere.density);
            printw("Engine Params\n");
            printw("\tSpeed:    %0.0f RPM,  %2.1f %%\n\tTorque: %5.2f %% (%0.0f HP)\n", e_rpm, e_rpm / 41730.0 * 100, W_TO_HP(c_W + t_W) / 10.0, W_TO_HP(c_W + t_W));  // HP_TO_TORQUE(W_TO_HP(c_W + t_W), e_rpm)

            printPVT("Inlet         ", &stage_inlet.out);
            printPVT("Compressor    ", &stage_compressor.out);
            printw("               W = %10.1f J (%5.0f HP)\n", c_W, c_W * 0.0013);
            printPVT("Bleed         ", &stage_bleed);
            printPVT("Combustor     ", &stage_combuster.out);
            printPVT("Turbine       ", &stage_turbine.out);
            printw("               W = %10.1f J (%5.0f HP)\n", t_W, t_W * 0.0013);
            // printPVT("Exhaust       ", &exhaust);

            refresh();
        }

        t += 1;
        tick();
    }
    // combustor
    // Assume perfect burn of fuel
    // Kerosene type BP Jet A-1, 43.15 MJ/kg, density at 15 °C is 804 kg/m3 (34.69 MJ/litre).
    // double combustionEnergy = fuelFlowKg * 43150000;  // J = kg * J/kg

    // 335-400 lbs
    // l45"xw20"xh26"
    // CCW 1591 RPM prop
    // 41730 rpm
    // -11, 1000 hp (shaft), 1100 wet hp, 0.530 lb/hp/hr SFC
    // start 770 EGT, takeoff 650 EGT
    // (-5, -6 1149 ITT, 923 ITT)
    // 106% RPM never exceed
    // ~10.55:1 compression
    // compress discharge ~1/3 used for fuel, 2/3 bypassed
    // turbine ~2/3 to power compressor

    // air flow 3.49266 kg/s

    // Ideal Brayton cycle:
    //     isentropic process – ambient air is drawn into the compressor, where it is pressurized.
    //     isobaric process – the compressed air then runs through a combustion chamber, where fuel is burned, heating that air—a constant-pressure process, since the chamber is open to flow in and out.
    //     isentropic process – the heated, pressurized air then gives up its energy, expanding through a turbine (or series of turbines). Some of the work extracted by the turbine is used to drive the compressor.
    //     isobaric process – heat rejection (in the atmosphere).
    // Actual Brayton cycle:
    //     adiabatic process – compression
    //     isobaric process – heat addition
    //     adiabatic process – expansion
    //     isobaric process – heat rejection

    // ideal gas law
    // P*V = n*R*T 
    // P pascal
    // V m^3
    // T kelvin
    // R ideal gas constant
    // n moles

    // adiabatic
    // PV^r = constant
    // r = adiabatic index
    // r = Cp / Cv = f + 2 / f
    // f = degrees of freedom = 7/5 diatomic (gass of N and O)
    // Cp = specific heat for constant pressure
    // Cv = specific heat for constant volume

    // P^1-rT^r = con
    // VT^f/2
    // TV^r-1
    // r = Cp/Cv = f + 2 / f
    // P-press, V-vol, T-temp, r-adiabatic index, f-degress of freedom (3-monatomic, 5-diatomic),
    // Cp-specific heat for constant pressure, Cv-specific heat for constant volume,

    // Moment Inertia
    // I = L / w  (I = kg m^2)
    // L - angular momentum, w - angular velocity,
    // r = I * a
    // r - applied torque, a - angular accel

    // ambient air
    // double c = PV^r;

    // AWI
    // Increae mass flow?
    // Reduce temperature?

    // compressor 1
    // compressor 2

    // combustor

    // turbine 1
    // turbine 2
    // turbine 3

    return 0;
}

void engineControl()
{
   int in = getch();

    switch (in)
    {
    case '!':
        e_ff -= 0.01;
        if (e_ff < 0.0) e_ff = 0.0;
        break;
    case '1':
        e_ff += 0.01;
        if (e_ff > 2.0) e_ff = 2.0;
        break;
    case '@':
        e_rpm -= 1000.0;
        if (e_rpm < 0.0) e_rpm = 0.0;
        break;
    case '2':
        e_rpm += 1000.0;
        if (e_rpm > 41730.0) e_rpm = 41730.0;
        break;
    case '#':
        atmosphere.density -= 0.01;
        if (e_rpm < 1.0) e_rpm = 1.0;
        break;
    case '3':
        atmosphere.density += 0.01;
        if (atmosphere.density > 1.5) atmosphere.density = 1.5;
        break;
    case '$':
        atmosphere.temperature -= 5.0;
        break;
    case '4':
        atmosphere.temperature += 5.0;
        break;
    case 'q':
        running = 0;
        break;
        case 'a': if (bleedOn > 0.001) bleedOn = 0.0; else bleedOn = 0.02; break;
        case 's': if(deiceOn > 0.001) deiceOn = 0.0; else deiceOn = 0.10; break;
        case 'd': if (awiOn > 0.001) awiOn = 0.0; else awiOn = 1.0; break;
    }
}

double lever_power = 0.001;
double lever_speed = 1.0;
int feather = 0;

void *tlnBackground(void *vargp)
{
    // Underspeed governor
    // * Beta mode, 71% - 97% RPM

    // Power Lever
    // * Flt Idle
    // * Meters more fuel than underspeed gov
    // Overspeed governor
    // F = m * a
    // Drag
    // Fd = 0.5 * p * v^2 * Cd * A
    // Engine Speed
    // -Fprop + -Ffriction + Ffuel
    // 1 hp = 745.69987158227 Nm/s
    // JET-A   46.36 MJ/kg, 36.86 MJ/l
    long t = 0;

    const int timeMS = 25;
    const double timeMSd = 0.025;

    double e_rpm = 0;

    double const ppc_max_pressure = 1;
    double const ppc_max_prate = 0.1;

    while (running)
    {
        // prop gov
        double pg_rpm_target = (lever_speed * 0.06) + 0.94; // 0.94-1.0, 1.07
        if (lever_power < 0)
        {
            pg_rpm_target = 1.07;
        }
        double pg_out_pilot_valve = fmax(-1, fmin(1, pid_update(&pg_out_pi, pg_rpm_target, e_rpm, timeMSd)));

        // Simplified prop pitch control
        const double ppc_min_area = 0.2;
        const double ppc_max_area = 10;
        double const ppc_angle_feather = 1;
        double const ppc_angle_flt_idle = 0.01;
        double const ppc_angle_gnd_idle = 0;
        double const ppc_angle_rev = 0.2;

        double ppc_beta_pressure = fmax(0, fmin(1, ppc_beta_pressure + (pg_out_pilot_valve * ppc_max_prate)) - feather);
        double ppc_prop_angle = 1 - ppc_beta_pressure;
        if (lever_power < 0)
        {
            double rev = fmin(-0.5, lever_power) * -2 + -1;
            if (rev)
            {
                ppc_prop_angle = fmax(ppc_prop_angle, ppc_angle_rev * rev);
            }
            else
            {
                double gnd = fmin(0, fmax(-0.5, lever_power)) * -2;
                ppc_prop_angle = fmax(ppc_prop_angle, ppc_angle_flt_idle - (ppc_angle_flt_idle * gnd));
            }
        }
        else
        {
            ppc_prop_angle = fmax(ppc_prop_angle, ppc_angle_flt_idle);
        }
        double ppc_prop_area = ppc_min_area + (ppc_prop_angle * ppc_max_area);

        // Simplified fuel control
        const double fc_flow_rate_max = 5;
        const double fc_flow_rate_min = 0.03;
        const double fc_flight_idle_adj = 0.0925;
        double fc_underspeed_target = (lever_speed * 0.26) + 0.71; // 0.71-0.97
        double fc_underspeed_valve = fmin(1, fmax(0, pid_update(&fc_underspeed_pi, fc_underspeed_target, e_rpm, timeMSd)));
        double fc_manual_fuel_valve = fmax(0, lever_power + fc_flight_idle_adj);

        double valve = fmax(fc_underspeed_valve, fc_manual_fuel_valve);

        double fc_fuel_flow = fmax(fc_flow_rate_min, valve) * fc_flow_rate_max;

        // Simplified engine model
        const double mass = 2;
        const double fuel_to_power_ratio = 0.3;
        double f_prop_drag = -0.5 * (e_rpm * e_rpm) * ppc_prop_area; // Fd = 0.5 * p * v^2 * Cd * A
        double f_fuel = fc_fuel_flow * fuel_to_power_ratio / (2 - e_rpm);
        e_rpm += (f_prop_drag + f_fuel) * timeMSd / mass;

        if (t % 4 == 0)
        {
            clear();
            printw("Power:       %0.2f     Speed:     %0.2f\n", lever_power, lever_speed);
            printw("Engine:        rpm   %5.1f%%  drag %f   torque %f   temp %5.1f\n", (e_rpm * 100), f_prop_drag, f_fuel, ((2 - e_rpm) * fc_fuel_flow * 120));
            printw("FC:                            ff % f   % f lbs/hr\n", fc_fuel_flow, (fc_fuel_flow * 128));
            printw("   Underspeed: t_rpm %5.1f%%     v % 0.4f    ", (fc_underspeed_target * 100), fc_underspeed_valve);
            pid_print(&fc_underspeed_pi);
            printw("\n");
            printw("   Manual:                      v % 0.4f\n", fc_manual_fuel_valve);
            printw("Prop Gov:      t_rpm %5.1f%%     v % 0.4f    ", (pg_rpm_target * 100), pg_out_pilot_valve);
            pid_print(&pg_out_pi);
            printw("\n");
            printw("Prop:                              %f  b %f a %f\n", ppc_prop_area, ppc_beta_pressure, ppc_prop_angle);
            printw("t = %d\n", t * 25l);
            refresh();
        }

        t += 1;

        static struct timespec ts;
        ts.tv_sec = timeMS / 1000;
        ts.tv_nsec = (timeMS % 1000) * 1000000;
        nanosleep(&ts, NULL);
    }
    return 0;
}

void tlnControl()
{
    int in = getch();

    switch (in)
    {
    case '1':
        lever_power = fmax(-1, lever_power - 0.01);
        break;
    case '2':
        lever_power = fmin(1, lever_power + 0.01);
        break;
    case '3':
        lever_speed = fmax(0, lever_speed - 0.1);
        break;
    case '4':
        lever_speed = fmin(1, lever_speed + 0.1);
        break;
    case 'f':
        feather = (feather + 1) % 2;
        break;
    case 'p':
        pg_out_pi.kp += 0.1;
        break;
    case 'P':
        pg_out_pi.kp -= 0.1;
        break;
    case 'i':
        pg_out_pi.ki += 0.1;
        break;
    case 'I':
        pg_out_pi.ki -= 0.1;
        break;

    case 'q':
        running = 0;
        break;
    }
}

int main(int argc, const char *argv[])
{
    initscr();
    cbreak();
    noecho();

    intrflush(stdscr, FALSE);
    keypad(stdscr, TRUE);

    pthread_t thread_id;
    //pthread_create(&thread_id, NULL, tlnBackground, NULL);
    pthread_create(&thread_id, NULL, engineBackground, NULL);

    while (running)
    {
        // tlnControl();
        engineControl();
    }

    pthread_join(thread_id, 0);

    endwin();
    pthread_exit(NULL);

    return 0;
}