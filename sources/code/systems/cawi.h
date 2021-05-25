/**
 * Theory
 *      Water/Ethonol Inject into compressor inlet
 *      Reduces temp, increases air mass density.
 *
 * Approaches
 * - Boost
 *      Only available at 100% throttle
 *      Higher fuel flow
 *
 * - Change Max Throttle
 *      Programmatically change max throtle from 1.0 to something higher
 *      Higher Fuel flow
 *
 * Condition
 * RPM > 90%
 * PL > 1" forward flt idle
 * Wter injection = cont
 * Effects
 * Pumps on
 * Bleed Air closes
 * AWI shutoff valve open
 * Immediate 30-35% trq increase
 *  SRL Signaled
 *
 * AWI continues until
 * switched off
 * fluid runs out
 * rpm < 90%
 */

#ifndef _CAWI_H_
#define _CAWI_H_

#include <stdlib.h>

// cawi_tank 16 gallons
// water_injection_switch (off, cont)
// awi_pump_test (no 1, center, no 2)
typedef enum { WATER_OFF = 0, WATER_CONT = 1 } SwitchWaterInjection;
typedef enum { PUMP_OFF = 0, PUMP_LEFT = 1, PUMP_RIGHT = 2 } SwitchPumpTest;

static XPLMDataRef cawiFluid = NULL;
static XPLMDataRef switchWaterInjection = NULL;
static XPLMDataRef switchTest = NULL;

void init();
void registerCallbacks();
void unregisterCallbacks();
float flightLoopCallback(float elapsedMe, float elapsedSim, int counter, void* refcon);

#endif /* _CAWI_H_ */