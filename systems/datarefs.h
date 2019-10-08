/**
 *
 */
#ifndef _DATAREFS_H_
#define _DATAREFS_H_

#include <XPLMDataAccess.h>
#include <XPLMPlugin.h>

#include <cstdlib>

#define MSG_ADD_DATAREF 0x01000000

#define DATAREFEDITOR_ADD(dataRefStr) \
    XPLMSendMessageToPlugin(datarefs::dataRefEditor, MSG_ADD_DATAREF, (void *)dataRefStr)

#define REGISTER_DATAACCESSOR_O_INT(dataRef, getCallback)                                                           \
    XPLMRegisterDataAccessor(dataRef, xplmType_Int, 0, getCallback, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, \
                             NULL, NULL, NULL, NULL, NULL)
#define REGISTER_DATAACCESSOR_IO_INT(dataRef, getCallback, setCallback)                                              \
    XPLMRegisterDataAccessor(dataRef, xplmType_Int, 1, getCallback, setCallback, NULL, NULL, NULL, NULL, NULL, NULL, \
                             NULL, NULL, NULL, NULL, NULL, NULL)

#define REGISTER_DATAACCESSOR_O_FLT(dataRef, getCallback)                                                           \
    XPLMRegisterDataAccessor(dataRef, xplmType_Float, 0, NULL, NULL, getCallback, NULL, NULL, NULL, NULL, NULL, NULL, \
                             NULL, NULL, NULL, NULL, NULL)

namespace datarefs {
// Metro Datarefs
static const char *sAnnuciatorNoseSteerStr = "sw4/annuciator/nose_steer";
static const char *sAnnuciatorNoseSteeringFailedStr = "sw4/annuciator/nose_steering_failed";

// X-Plane Datarefs

// sim/cockpit2/controls/tailwheel_lock_ratio
// sim/flightmodel2/gear/tire_steer_command_deg float[gear]
// sim/flightmodel2/gear/tire_steer_actual_deg float[gear]
// static const char* sXpOverrideThrottleStr    = "sim/operations/override/override_throttles";

static const char *sSimCockpit2ControlsGearHandleDownStr = "sim/cockpit2/controls/gear_handle_down";
static const char *sSimCockpit2ContorlsNosewheelSteerOnStr = "sim/cockpit2/controls/nosewheel_steer_on";
static const char *sSimCockpit2ControlsTotalHeadingRatioStr = "sim/cockpit2/controls/total_heading_ratio";
static const char *sSimCockpit2EngineActuatorsPropRatioStr = "sim/cockpit2/engine/actuators/prop_ratio";  // [n]
static const char *sSimCockpit2EngineActuatorsPropRotSpeedRadSecStr  = "sim/cockpit2/engine/actuators/prop_rotation_speed_rad_sec";
static const char *sSimCockpit2HydraulicsIndicatorStr = "sim/cockpit2/hydraulics/indicators/hydraulic_pressure_1";
static const char *sSimOperationsOverrideWheelSteerStr = "sim/operation/override/override_wheel_steer";



static const char *sSimFlightmodel2GearTireSteerCommandDegStr =
    "sim/flightmodel2/gear/tire_steer_command_deg";  // float[gear]
// sim/flightmodel/parts/tire_steer_act[n]

static XPLMPluginID dataRefEditor = 0;

}  // namespace datarefs

#endif  // _DATAREFS_H_