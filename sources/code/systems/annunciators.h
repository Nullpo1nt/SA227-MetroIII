/**
 *
 * L ENG FIRE           sim/cockpit2/annunciators/engine_fires
 * R ENG FIRE           sim/cockpit2/annunciators/engine_fires
 * CABIN DOOR           sim/cockpit/warnings/annunciators/cabin_door_open
 * L BETA               sim/cockpit2/annunciators/beta or sim/cockpit2/annunciators/prop_beta
 * R BETA               sim/cockpit2/annunciators/beta
 * LOW SUCTION          sim/cockpit2/annunciators/low_vacuum
 * L INTAKE HT
 * R INTAKE HT
 * =
 * L WING OVHT
 * R WING OVHT
 * BATTERY FAULT
 * L CHIP DET           sim/cockpit/warnings/annunciators/chip_detect
 * R CHIP DET           sim/cockpit/warnings/annunciators/chip_detect
 * CABIN ALTITUDE       sim/cockpit/warnings/annunciators/cabin_altitude_12500
 * L W/S HT
 * R W/S HT
 * =
 * L OIL PRESSURE       sim/cockpit/warnings/annunciators/oil_pressure
 * R OIL PRESSURE       sim/cockpit/warnings/annunciators/oil_pressure
 * SAS FAULT
 * L XFER PUMP
 * R XFER PUMP
 * GPU PLUG IN          sim/cockpit/warnings/annunciators/external_power_on
 * -
 * -
 * =
 * L HYDR PRESSURE
 * R HYDR PRESS
 * CARGO DOOR
 * L BAT DISC
 * R BAT DISC
 * L SRL OFF
 * SAS ARM
 * SAS DEICE
 * =
 * -
 * -
 * -
 * L AC BUS
 * R AC BUS
 * R SRL OFF
 * AWI NO. 1 PUMP ON
 * AWI NO. 2 PUMP ON
 * =
 * -
 * GEAR DOOR POSITION
 * -
 * L GENERATOR FAIL         sim/cockpit2/annunciators/generator_off
 * R GENERATOR FAIL         sim/cockpit2/annunciators/generator_off
 * ANTI-SKID
 * NOSE STEERING
 * =
 *
 *
 * BYPASS OPEN L
 * BYPASS OPEN R
 * NOSE STEER FAIL
 * L FUEL
 * L HYD
 * X-FLOW OPEN
 * R FUEL
 * R HYD
 * L DUCT HEAT CYCL
 * R HEAT CYCL
 * L ENG FIRE, E, OK   sim/cockpit/warnings/annunciators/engine_fire
 * R ENG FIRE, E, OK
 *
 * DOOR UNSAFE
 * SWITCHES NORMAL
 */
#ifndef _ANNUCIATORS_H_
#define _ANNUCIATORS_H_

#include <XPLMDataAccess.h>
#include <stdbool.h>
#include <stdlib.h>

typedef enum {
    // 0
    ANNUNC_NWS,
    ANNUNC_L_ENG_FIRE,
    ANNUNC_R_ENG_FIRE,
    ANNUNC_CABIN_DOOR,
    ANNUNC_L_BETA,
    // 5
    ANNUNC_R_BETA,
    ANNUNC_LOW_SUCTION,
    ANNUNC_L_INTAKE_HT,
    ANNUNC_R_INTAKE_HT,
    ANNUNC_L_WING_OVHT,
    // 10
    ANNUNC_R_WING_OVHT,
    ANNUNC_BATTERY_FAULT,
    ANNUNC_L_CHIP_DET,
    ANNUNC_R_CHIP_DET,
    ANNUNC_CABIN_ALTITUDE,
    // 15
    ANNUNC_L_WS_HT,
    ANNUNC_R_WS_HT,
    ANNUNC_L_OIL_PRESSURE,
    ANNUNC_R_OIL_PRESSURE,
    ANNUNC_SAS_FAULT,
    // 20
    ANNUNC_L_XFER_PUMP,
    ANNUNC_R_XFER_PUMP,
    ANNUNC_GPU_PLUG_IN,
    ANNUNC_L_HYDR_PRESSURE,
    ANNUNC_R_HYDR_PRESSURE,
    // 25
    ANNUNC_CARGO_DOOR,
    ANNUNC_L_BAT_DISC,
    ANNUNC_R_BAT_DISC,
    ANNUNC_L_SRL_OFF,
    ANNUNC_SAS_ARM,
    // 30
    ANNUNC_SAS_DEICE,
    ANNUNC_L_AC_BUS,
    ANNUNC_R_AC_BUS,
    ANNUNC_R_SRL_OFF,
    ANNUNC_AWI_1_PUMP_ON,
    // 35
    ANNUNC_AWI_2_PUMP_ON,
    ANNUNC_GEAR_DOOR_POSITION,
    ANNUNC_L_GENERATOR_FAIL,
    ANNUNC_R_GENERATOR_FAIL,
    ANNUNC_ANTISKID,
    // 40
    ANNUNC_NOSE_STEERING,
    ANNUNC_BYPASS_OPEN_L,
    ANNUNC_BYPASS_OPEN_R,
    ANNUNC_NOSE_STEER_FAIL,
    ANNUNC_L_FUEL,
    // 45
    ANNUNC_L_HYD,
    ANNUNC_XFLOW_OPEN,
    ANNUNC_R_FUEL,
    ANNUNC_R_HYD,
    ANNUNC_L_DUCT_HEAT_CYCL,
    // 50
    ANNUNC_R_HEAT_CYCL,
    ANNUNC_L_ENG_FIRE_E,
    ANNUNC_L_ENG_FIRE_OK,
    ANNUNC_R_ENG_FIRE_E,
    ANNUNC_R_ENG_FIRE_OK,
    // 55
    ANNUNC_DOOR_UNSAFE,
    ANNUNC_SWITCHES_NORMAL
} annunc_t;

typedef enum { ANNUC_OFF = 0, ANNUC_ON = 1, ANNUNC_FAILED = 2 } annunc_state_t;

typedef struct annunciator_s {
    annunc_state_t annunciators[57];
    bool switch_test;
} annunciator_t;

void annunciator_set(annunciator_t *annunc, annunc_t annunciator, annunc_state_t state);

void annunciator_set_switch_test(annunciator_t *annunc, bool state) {}

void update() {}

// sAnnuciatorNoseSteerDataRef = XPLMRegisterDataAccessor(
//     sAnnuciatorNoseSteerStr,
//     xplmType_Int,                                  // The types we support
//     0,                                             // Not writable
//     getAnnuciatorNoseSteerCallback,               // Integer get accessor
//     NULL,                                          // Integer set accessor (none)
//     NULL, NULL,                                    // Float accessors
//     NULL, NULL,                                    // Doubles accessors
//     NULL, NULL,                                    // Int array accessors
//     NULL, NULL,                                    // Float array accessors
//     NULL, NULL,                                    // Raw data accessors
//     NULL, NULL);

// XPLMUnregisterDataAccessor(sAnnuciatorNoseSteerDataRef);
//     XPLMUnregisterDataAccessor(sAnnuciatorNoseSteeringFailedDataRef);

// static int getAnnuciatorNoseSteerCallback(void *inRefcon)
// {
//     return nwsAnnuciatorNoseSteer;
// }

// static int getAnnunciatorNoseSteeringFail(void *inRefcon)
// {
//     return nwsAnnuciatorNoseSteeringFailed;
// }

#endif /* _ANNUCIATORS_H_ */