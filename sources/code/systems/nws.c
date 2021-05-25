#include "nws.h"

#include <XPLMDataAccess.h>
#include <XPLMProcessing.h>
#include <XPLMUtilities.h>
#include <stdbool.h>
#include <stdlib.h>

#include "commands.h"
#include "datarefs.h"

typedef enum { NWS_ARM_OFF = 0, NWS_ARM_ARMED = 1, NWS_ARM_TEST = 2 } NWS_SwitchArm_t;
typedef enum { NWS_PARK_OFF = 0, NWS_PARK_ON = 1 } NWS_SwitchPark_t;
typedef enum { NWS_ENGAGE_OFF = 0, NWS_ENGAGE_ON = 1 } NWS_SwitchEngage_t;

const char *annunciatorNwsStr = "sw4/annunciator/nws";

const char *lightParkStr = "sw4/cockpit/nws_park_light";

const char *switchArmStr = "sw4/cockpit/nws_arm";
const char *switchParkStr = "sw4/cockpit/nws_park";
const char *switchEngageStr = "sw4/cockpit/nws_engage";
// const char* switchTestStr = "sw4/cockpit/nws_test";

const char *commandEngageStr = "sw4/commands/nws_engage";
const char *commandEngageDescStr = "Nose wheel steering engage momentary button.";
const char *commandParkStr = "sw4/commands/nws_park";
const char *commandParkDescStr = "Park (increased steer authority) momentary button.";

/**
 * Rate at which authority can change between min and max.
 */
const float NWS_AUTHORITY_DELTA_DEGSEC = 5.0f;
const float NWS_AUTHORITY_MAX_DEG = 63.0f;
const float NWS_AUTHORITY_MIN_DEG = 12.0f;

const float NWS_STEER_DELTA_DEGSEC = 0.5f;

const float NWS_RIGHT_POWERLEVER_SWITCH = 117.0f;
// const float NWS_MAX_PRESSURE = 2000.0f;

XPLMDataRef gearHandleDownDataRef = NULL;
XPLMDataRef hydraulicPressure = NULL;
XPLMDataRef nwsOverride = NULL;

// XP Datarefs
XPLMDataRef simCockpit2ContorlsNosewheelSteerOnDataRef = NULL;
XPLMDataRef simCockpit2ControlsTotalHeadingRatioDataRef = NULL;
XPLMDataRef simCockpit2EngineActuatorsPropRotSpeedRadSecDataRef = NULL;
XPLMDataRef simFlightmodel2GearTireSteerCommandDegDataRef = NULL;
// XPLMDataRef simOperationsOverrideWheelSteerDatRef = NULL;

XPLMDataRef annunciatorNwsDataRef = NULL;

XPLMDataRef switchArmDataRef = NULL;
XPLMDataRef switchParkDataRef = NULL;
XPLMDataRef switchEngageDataRef = NULL;
// XPLMDataRef switchTestDataRef = NULL;

XPLMDataRef lightParkDataRef = NULL;

XPLMCommandRef commandEngage = NULL;
XPLMCommandRef commandPark = NULL;
// XPLMCommandRef commandNws = NULL;

struct nws_data_s {
    bool annunciator;
    float light_park;  // 0..1 Park light intensity

    NWS_SwitchArm_t switch_arm;        // Switch positions
    NWS_SwitchPark_t switch_park;      //
    NWS_SwitchEngage_t switch_engage;  //

    float authority_deg;  // This represents the total deflection authority the NWS can
                          // turn the nose wheel.
    float steer_angle_deg;
} g_nws_data = {  //
    .annunciator = false,
    .light_park = 0.0f,
    .switch_arm = NWS_ARM_OFF,
    .switch_park = NWS_PARK_OFF,
    .switch_engage = NWS_ENGAGE_OFF,
    .authority_deg = NWS_AUTHORITY_MIN_DEG,
    .steer_angle_deg = 0.0f};

/**
 * Callbacks
 */

int getAnnunciatorNws(void *inRefcon) { return g_nws_data.annunciator; }

float getLightPark(void *inRefcon) { return g_nws_data.light_park; }

int getSwitchArmCallback(void *inRefcon) { return g_nws_data.switch_arm; }
void setSwitchArmCallback(void *inRefcon, int value) {
    switch (value) {
        case 0:
            g_nws_data.switch_arm = NWS_ARM_OFF;
            break;
        case 1:
            g_nws_data.switch_arm = NWS_ARM_ARMED;
            break;
        case 2:
            g_nws_data.switch_arm = NWS_ARM_TEST;
            break;
        default:
            g_nws_data.switch_arm = NWS_ARM_OFF;
    }
}

int getSwitchParkCallback(void *inRefcon) { return g_nws_data.switch_park; }
void setSwitchParkCallback(void *inRefcon, int value) {
    switch (value) {
        case 0:
            g_nws_data.switch_park = NWS_PARK_OFF;
            break;
        case 1:
            g_nws_data.switch_park = NWS_PARK_ON;
            break;
        default:
            g_nws_data.switch_park = NWS_PARK_OFF;
    }
}

int getSwitchEngageCallback(void *inRefcon) { return g_nws_data.switch_engage; }
void setSwitchEngageCallback(void *inRefcon, int value) {
    switch (value) {
        case 0:
            g_nws_data.switch_engage = NWS_ENGAGE_OFF;
            break;
        case 1:
            g_nws_data.switch_engage = NWS_ENGAGE_ON;
            break;
        default:
            g_nws_data.switch_engage = NWS_ENGAGE_OFF;
    }
}

int commandEngageCallback(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon) {
    switch (inPhase) {
        case xplm_CommandBegin:
            XPLMSetDatai(switchEngageDataRef, NWS_ENGAGE_ON);
            break;
        case xplm_CommandEnd:
            XPLMSetDatai(switchEngageDataRef, NWS_ENGAGE_OFF);
            break;
    }

    return 0;
}

int commandParkCallback(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon) {
    switch (inPhase) {
        case xplm_CommandBegin:
            XPLMSetDatai(switchParkDataRef, NWS_PARK_ON);
            break;
        case xplm_CommandEnd:
            XPLMSetDatai(switchParkDataRef, NWS_PARK_OFF);
            break;
    }

    return 0;
}

void nws_init() {
    // XPlane Datarefs
    gearHandleDownDataRef = XPLMFindDataRef(sSimCockpit2ControlsGearHandleDownStr);
    hydraulicPressure = XPLMFindDataRef(sSimCockpit2HydraulicsIndicatorStr);
    simCockpit2ControlsTotalHeadingRatioDataRef = XPLMFindDataRef(sSimCockpit2ControlsTotalHeadingRatioStr);
    simCockpit2EngineActuatorsPropRotSpeedRadSecDataRef =
        XPLMFindDataRef(sSimCockpit2EngineActuatorsPropRotSpeedRadSecStr);
    simFlightmodel2GearTireSteerCommandDegDataRef = XPLMFindDataRef(sSimFlightmodel2GearTireSteerCommandDegStr);
    simCockpit2ContorlsNosewheelSteerOnDataRef = XPLMFindDataRef(sSimCockpit2ContorlsNosewheelSteerOnStr);

    nwsOverride = XPLMFindDataRef(sSimOperationsOverrideWheelSteerStr);
}

void nws_registerCallbacks() {
    XPLMSetDatai(nwsOverride, 1);

    annunciatorNwsDataRef = REGISTER_DATAACCESSOR_O_INT(annunciatorNwsStr, getAnnunciatorNws);
    lightParkDataRef = REGISTER_DATAACCESSOR_O_FLT(lightParkStr, getLightPark);
    switchArmDataRef = REGISTER_DATAACCESSOR_IO_INT(switchArmStr, getSwitchArmCallback, setSwitchArmCallback);
    switchParkDataRef = REGISTER_DATAACCESSOR_IO_INT(switchParkStr, getSwitchParkCallback, setSwitchParkCallback);
    switchEngageDataRef =
        REGISTER_DATAACCESSOR_IO_INT(switchEngageStr, getSwitchEngageCallback, setSwitchEngageCallback);
    // switchTestDataRef = XPLMFindDataRef(switchTestStr);

    // Commands
    commandEngage = XPLMCreateCommand(commandEngageStr, commandEngageDescStr);
    commandPark = XPLMCreateCommand(commandParkStr, commandParkDescStr);
    // commandNws = XPLMFindCommand("");

    REGISTER_CMD(commandEngage, commandEngageCallback, true);
    REGISTER_CMD(commandPark, commandParkCallback, true);

    // Override Nose wheel steer toggle in XPlane
    // REGISTER_CMD(commandNws, commandEngageCallback, true);

    XPLMRegisterFlightLoopCallback(nws_flightLoopCallback, -2, NULL);

    // Register dataref in editor
    // DATAREFEDITOR_ADD(switchArmStr);
    // DATAREFEDITOR_ADD(switchParkStr);
    // DATAREFEDITOR_ADD(switchEngageStr);
}

void nws_unregisterCallbacks() {
    XPLMUnregisterFlightLoopCallback(nws_flightLoopCallback, NULL);

    UNREGISTER_CMD(commandEngage, commandEngageCallback);
    UNREGISTER_CMD(commandPark, commandParkCallback);
    // UNREGISTER_CMD(commandNws, commandEngageCallback);

    XPLMUnregisterDataAccessor(switchArmDataRef);
    XPLMUnregisterDataAccessor(switchParkDataRef);
    XPLMUnregisterDataAccessor(switchEngageDataRef);
    // XPLMUnregisterDataAccessor(switchTestDataRef);
    XPLMUnregisterDataAccessor(lightParkDataRef);
    XPLMUnregisterDataAccessor(annunciatorNwsDataRef);

    XPLMSetDatai(nwsOverride, 0);
}

/**
 * Check if the power level is at mininum.
 */
inline bool isPowerLeverAtMinimum() {
    float rightPowerLevelPosition[1];
    XPLMGetDatavf(simCockpit2EngineActuatorsPropRotSpeedRadSecDataRef, rightPowerLevelPosition, 1, 1);
    return rightPowerLevelPosition[0] < NWS_RIGHT_POWERLEVER_SWITCH;
}

inline void setSteerCommand(const float steer) {
    float nws_steer_deg[1];
    nws_steer_deg[0] = g_nws_data.steer_angle_deg;
    XPLMSetDatavf(simFlightmodel2GearTireSteerCommandDegDataRef, nws_steer_deg, 0, 1);
}

/**
 * Calculate the authority delta for the time elapsed.
 */
inline float getAuthorityDelta(const float time) { return NWS_AUTHORITY_DELTA_DEGSEC * time; }
inline void updateParkLight() { g_nws_data.light_park = g_nws_data.authority_deg / NWS_AUTHORITY_MAX_DEG; }

void updateAuthority(const float timeDelta) {
    if (g_nws_data.switch_park == NWS_PARK_ON && g_nws_data.authority_deg < NWS_AUTHORITY_MAX_DEG) {
        g_nws_data.authority_deg += getAuthorityDelta(timeDelta);

        if (g_nws_data.authority_deg > NWS_AUTHORITY_MAX_DEG) {
            g_nws_data.authority_deg = NWS_AUTHORITY_MAX_DEG;
        }
        updateParkLight();
    } else if (g_nws_data.switch_park == NWS_PARK_OFF && g_nws_data.authority_deg > NWS_AUTHORITY_MIN_DEG) {
        g_nws_data.authority_deg -= getAuthorityDelta(timeDelta);

        if (g_nws_data.authority_deg < NWS_AUTHORITY_MIN_DEG) {
            g_nws_data.authority_deg = NWS_AUTHORITY_MIN_DEG;
        }
        updateParkLight();
    }
}

float nws_flightLoopCallback(float elapsedMe, float elapsedSim, int counter, void *refcon) {
    // Check for failure conditions
    // non-essential bus powered
    // const float pressure = XPLMGetDataf(hydraulicPressure) / maxPressure;
    // if (noseActual > 3 deg of commanded(left / right)) {
    // setNoseSteering(FLASH);
    // }

    float commandedSteerDeg = 0.0f;
    float nextUpdate = 5.0f;

    const bool gearDown = XPLMGetDatai(gearHandleDownDataRef);

    if (gearDown && g_nws_data.switch_arm == NWS_ARM_ARMED) {
        nextUpdate = -10.0f;

        // Update steer authority
        updateAuthority(1.0f);

        // Check for enabled conditions
        if (g_nws_data.switch_engage == NWS_ENGAGE_ON || isPowerLeverAtMinimum()) {
            XPLMSetDatai(simCockpit2ContorlsNosewheelSteerOnDataRef, 1);
            g_nws_data.annunciator = true;
            commandedSteerDeg =
                XPLMGetDataf(simCockpit2ControlsTotalHeadingRatioDataRef) * g_nws_data.authority_deg * 5.0f;
        }
    } else {
        g_nws_data.annunciator = false;
        XPLMSetDatai(simCockpit2ContorlsNosewheelSteerOnDataRef, 0);
    }

    const float cmdSteerVsSteer = commandedSteerDeg - g_nws_data.steer_angle_deg;
    if (cmdSteerVsSteer < 0.6f && cmdSteerVsSteer > -0.6f) {
        g_nws_data.steer_angle_deg = commandedSteerDeg;
        setSteerCommand(g_nws_data.steer_angle_deg);
    } else if (commandedSteerDeg < g_nws_data.steer_angle_deg) {
        g_nws_data.steer_angle_deg -= NWS_STEER_DELTA_DEGSEC;
        nextUpdate = -2.0f;
        setSteerCommand(g_nws_data.steer_angle_deg);
    } else if (commandedSteerDeg > g_nws_data.steer_angle_deg) {
        g_nws_data.steer_angle_deg += NWS_STEER_DELTA_DEGSEC;
        nextUpdate = -2.0f;
        setSteerCommand(g_nws_data.steer_angle_deg);
    }

    return nextUpdate;
}
