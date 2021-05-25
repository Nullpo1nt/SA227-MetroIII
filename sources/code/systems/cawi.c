#include "cawi.h"

#include <stdbool.h>

#include "datarefs.h";

XPLMDataRef cawiFluidDataRef = NULL;
XPLMDataRef switchWaterInjectionDataRef = NULL;
XPLMDataRef switchTest = NULL;

const float flow_lbs_sec = (2.6f * 6.3f) / 60.0f;
const float max_cawi_fluid = 128.0f;

bool annunciatorPumpLeft = false;
bool annunciatorPumpRight = false;

float cawiFluidLbs = max_cawi_fluid;
bool switchWaterInjection = false;

float flightLoopCallback(float elapsedMe, float elapsedSim, int counter, void* refcon) {
    if (switchWaterInjection) {
        if (rpm > 0.90 && PL > 1 inch) {
            bleedAirOffL;
            bleedAirOffR;

            if (cawiFluidLbs > 0.0f) {
                maxPower = 1.2f;
                cawiFluidLbs -= flow_lbs_sec * 1.0f;

                if (cawiFluidLbs < 0.0f) {
                    cawiFluidLbs = 0.0f;
                }
            } else {
                maxPower = 1.0f;
            }
        } else {
            maxPower = 1.0f;
        }
    }

    float nextUpdate = 0;
}