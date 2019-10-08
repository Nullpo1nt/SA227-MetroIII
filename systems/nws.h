/**
 * Nose Wheel Steering System
 *
 * Overrides X-Planes steering with custom logic.  Two modes are provided, free castoring and authority mode.
 *
 * In free castoring the nose wheel is allowed to castor to a max deflection of 63 degrees (as set in the model).
 *
 * In authority mode the wheel is commanded to a angle relative to the rudder deflection.  Authority starts at 10
 * degrees max and can be momentarily boosted to max deflection of 63 degrees using the park command.
 *
 * Authority mode is engaged when the NWS system is activated and either the NWS engage switch is depressed or the
 * right power lever is set to min RPM.
 *
 * Uses:
 * - Non-essential bus (28.5v)
 * - Hydraulically Actuated
 *
 * Requires:
 * - Landing Gear Down Position
 * - Rudder Pedal Deflection
 *
 *
 * Normal - Max 10" Left/Right
 * Variable Authority - 63-deg left/right
 *
 * After take off mechanical cam centers nose gear
 *
 * Annunciator - NOSE STEERING (green) when armed
 * Flashing nose wheel turned more than 3" from commanded
 * NOSE STEER FAIL
 * Hyd pressure available to actuator, but system not engaged, or not armed
 * not present if antiskid system install, replaced by square amber light on panel
 *
 * Switches
 *
 * - Speed Lever Minimum switch
 * - Throttle Engage Button
 *   Nose steer engaged if right speed lever fully aft or throttle button depressed
 *
 * - Test
 *   Left - Centered - Right
 *
 * - Park
 *   Momentary, defaults to centered
 *   Depressing and hold incease authority, several seconds inceases to 63-deg
 *   During this time the PARK button illuminates with a brilliance in proportion to the increased authority. When the
 *   button is released, authority and brilliance both decrease over a period of several seconds.
 *
 * - Arm
 *   Armed - off - Valve Test
 *
 * System Documentation:
 * - Merlin IVC/Metro III Pilot Training Manual, Revision No. 5, 1992, Chapter 14 Landing Gear and Brakes
 * - Airplane Flight Manual, Fairchild Aircraft Model SA227-AC, 16000 Pounds (AFM 8AC), Section 6 Manufacturer's Data,
 * 6-70
 */
#ifndef _NWS_H_
#define _NWS_H_

#include <XPLMDataAccess.h>
#include <XPLMProcessing.h>
#include <XPLMUtilities.h>

#include "commands.h"
#include "datarefs.h"

#include <cstdlib>

namespace nws {

enum SwitchArm : int { ARM_OFF = 0, ARM_ARMED = 1, ARM_TEST = 2 };
enum SwitchPark : int { PARK_OFF = 0, PARK_ON = 1 };
enum SwitchEngage : int { ENGAGE_OFF = 0, ENGAGE_ON = 1 };

void init();
void registerCallbacks();
void unregisterCallbacks();
float flightLoopCallback(float elapsedMe, float elapsedSim, int counter, void* refcon);

}  // namespace nws

#endif /* _NWS_H_ */