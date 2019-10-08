#include "nws.h"

namespace nws {


constexpr char* annunciatorNwsStr = "sw4/annunciator/nws";

const char* lightParkStr = "sw4/cockpit/nws_park_light";

const char* switchArmStr = "sw4/cockpit/nws_arm";
const char* switchParkStr = "sw4/cockpit/nws_park";
const char* switchEngageStr = "sw4/cockpit/nws_engage";
// const char* switchTestStr = "sw4/cockpit/nws_test";

const char* commandEngageStr = "sw4/commands/nws_engage";
const char* commandEngageDescStr = "Nose wheel steering engage momentary button.";
const char* commandParkStr = "sw4/commands/nws_park";
const char* commandParkDescStr = "Park (increased steer authority) momentary button.";



/**
 * Rate at which authority can change between min and max.
 */
constexpr float nws_authority_deltaDegSec = 5.0f;
constexpr float nws_authority_maxDeg = 63.0f;
constexpr float nws_authority_minDeg = 12.0f;

constexpr float nws_steer_deltaDegSec = 0.5f;

const float nws_right_powerlever_switch = 117.0f;
// const float maxPressure = 2000.0f;

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


bool annunciatorNws = false;

float lightPark = 0.0f;

SwitchArm armSwitch = ARM_OFF;
SwitchPark parkSwitch = PARK_OFF;
SwitchEngage engageSwitch = ENGAGE_OFF;

// This represents the total deflection authority the NWS can
// turn the nose wheel.

float authority = nws_authority_minDeg;
float steerAng = 0.0f;


/**
 * Callbacks
 */

int getAnnunciatorNws(void *inRefcon) { return annunciatorNws; }

float getLightPark(void *inRefcon) { return lightPark; }

int getSwitchArmCallback(void *inRefcon) { return armSwitch; }
void setSwitchArmCallback(void *inRefcon, int value) {
    switch (value) {
        case 0:
            armSwitch = ARM_OFF;
            break;
        case 1:
            armSwitch = ARM_ARMED;
            break;
        case 2:
            armSwitch = ARM_TEST;
            break;
        default:
            armSwitch = ARM_OFF;
    }
}

int getSwitchParkCallback(void *inRefcon) { return parkSwitch; }
void setSwitchParkCallback(void *inRefcon, int value) {
    switch (value) {
        case 0:
            parkSwitch = PARK_OFF;
            break;
        case 1:
            parkSwitch = PARK_ON;
            break;
        default:
            parkSwitch = PARK_OFF;
    }
}

int getSwitchEngageCallback(void *inRefcon) { return engageSwitch; }
void setSwitchEngageCallback(void *inRefcon, int value) {
    switch (value) {
        case 0:
            engageSwitch = ENGAGE_OFF;
            break;
        case 1:
            engageSwitch = ENGAGE_ON;
            break;
        default:
            engageSwitch = ENGAGE_OFF;
    }
}

int commandEngageCallback(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon) {
    switch (inPhase) {
        case xplm_CommandBegin:
            XPLMSetDatai(switchEngageDataRef, ENGAGE_ON);
            break;
        case xplm_CommandEnd:
            XPLMSetDatai(switchEngageDataRef, ENGAGE_OFF);
            break;
    }

    return 0;
}

int commandParkCallback(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon) {
    switch (inPhase) {
        case xplm_CommandBegin:
            XPLMSetDatai(switchParkDataRef, PARK_ON);
            break;
        case xplm_CommandEnd:
            XPLMSetDatai(switchParkDataRef, PARK_OFF);
            break;
    }

    return 0;
}

void init() {
    // XPlane Datarefs
    gearHandleDownDataRef = XPLMFindDataRef(datarefs::sSimCockpit2ControlsGearHandleDownStr);
    hydraulicPressure = XPLMFindDataRef(datarefs::sSimCockpit2HydraulicsIndicatorStr);
    simCockpit2ControlsTotalHeadingRatioDataRef = XPLMFindDataRef(datarefs::sSimCockpit2ControlsTotalHeadingRatioStr);
    simCockpit2EngineActuatorsPropRotSpeedRadSecDataRef = XPLMFindDataRef(datarefs::sSimCockpit2EngineActuatorsPropRotSpeedRadSecStr);
    simFlightmodel2GearTireSteerCommandDegDataRef =
        XPLMFindDataRef(datarefs::sSimFlightmodel2GearTireSteerCommandDegStr);
    simCockpit2ContorlsNosewheelSteerOnDataRef = XPLMFindDataRef(datarefs::sSimCockpit2ContorlsNosewheelSteerOnStr);

    nwsOverride = XPLMFindDataRef(datarefs::sSimOperationsOverrideWheelSteerStr);
}

void registerCallbacks() {
    XPLMSetDatai(nwsOverride, 1);

    annunciatorNwsDataRef = REGISTER_DATAACCESSOR_O_INT(annunciatorNwsStr, getAnnunciatorNws);
    lightParkDataRef      = REGISTER_DATAACCESSOR_O_FLT(lightParkStr, getLightPark);
    switchArmDataRef      = REGISTER_DATAACCESSOR_IO_INT(switchArmStr, getSwitchArmCallback, setSwitchArmCallback);
    switchParkDataRef     = REGISTER_DATAACCESSOR_IO_INT(switchParkStr, getSwitchParkCallback, setSwitchParkCallback);
    switchEngageDataRef   = REGISTER_DATAACCESSOR_IO_INT(switchEngageStr, getSwitchEngageCallback, setSwitchEngageCallback);
    // switchTestDataRef = XPLMFindDataRef(switchTestStr);

    // Commands
    commandEngage = XPLMCreateCommand(commandEngageStr, commandEngageDescStr);
    commandPark   = XPLMCreateCommand(commandParkStr,   commandParkDescStr);
    // commandNws = XPLMFindCommand("");

    REGISTER_CMD(commandEngage, commandEngageCallback, true);
    REGISTER_CMD(commandPark,   commandParkCallback,   true);

    // Override Nose wheel steer toggle in XPlane
    // REGISTER_CMD(commandNws, commandEngageCallback, true);

    XPLMRegisterFlightLoopCallback(flightLoopCallback, -2, NULL);

    // Register dataref in editor
    // DATAREFEDITOR_ADD(switchArmStr);
    // DATAREFEDITOR_ADD(switchParkStr);
    // DATAREFEDITOR_ADD(switchEngageStr);
}

void unregisterCallbacks() {
    XPLMUnregisterFlightLoopCallback(flightLoopCallback, NULL);

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
    return rightPowerLevelPosition[0] < nws_right_powerlever_switch;
}

inline void setSteerCommand(const float steer) {
    float nws_steer_deg[1];
    nws_steer_deg[0] = steerAng;
    XPLMSetDatavf(simFlightmodel2GearTireSteerCommandDegDataRef, nws_steer_deg, 0, 1);
}

/**
 * Calculate the authority delta for the time elapsed.
 */
inline float getAuthorityDelta(const float time) { return nws_authority_deltaDegSec * time; }
inline void updateParkLight() { lightPark = authority / nws_authority_maxDeg; }

void updateAuthority(const float timeDelta) {
    if (parkSwitch == PARK_ON && authority < nws_authority_maxDeg) {
        authority += getAuthorityDelta(timeDelta);

        if (authority > nws_authority_maxDeg) {
            authority = nws_authority_maxDeg;
        }
        updateParkLight();
    } else if (parkSwitch == PARK_OFF && authority > nws_authority_minDeg) {
        authority -= getAuthorityDelta(timeDelta);

        if (authority < nws_authority_minDeg) {
            authority = nws_authority_minDeg;
        }
        updateParkLight();
    }

    
}

float flightLoopCallback(float elapsedMe, float elapsedSim, int counter, void *refcon) {
    // Check for failure conditions
    // non-essential bus powered
    // const float pressure = XPLMGetDataf(hydraulicPressure) / maxPressure;
    // if (noseActual > 3 deg of commanded(left / right)) {
    // setNoseSteering(FLASH);
    // }

    float commandedSteerDeg = 0.0f;
    float nextUpdate = 5.0f;

    const bool gearDown = XPLMGetDatai(gearHandleDownDataRef);

    if (gearDown && armSwitch == ARM_ARMED) {
        nextUpdate = -10.0f;

        // Update steer authority
        updateAuthority(1.0f);

        // Check for enabled conditions
        if (engageSwitch == ENGAGE_ON || isPowerLeverAtMinimum()) {
            XPLMSetDatai(simCockpit2ContorlsNosewheelSteerOnDataRef, 1);
            annunciatorNws = true;
            commandedSteerDeg = XPLMGetDataf(simCockpit2ControlsTotalHeadingRatioDataRef) * authority * 5.0f;
        }
    } else {
        annunciatorNws = false;
        XPLMSetDatai(simCockpit2ContorlsNosewheelSteerOnDataRef, 0);
    }

    const float cmdSteerVsSteer = commandedSteerDeg - steerAng;
    if (cmdSteerVsSteer < 0.6f && cmdSteerVsSteer > -0.6f) {
        steerAng = commandedSteerDeg;
        setSteerCommand(steerAng);
    } else if (commandedSteerDeg < steerAng) {
        steerAng -= nws_steer_deltaDegSec;
        nextUpdate = -2.0f;
        setSteerCommand(steerAng);
    } else if (commandedSteerDeg > steerAng) {
        steerAng += nws_steer_deltaDegSec;
        nextUpdate = -2.0f;
        setSteerCommand(steerAng);
    }

    return nextUpdate;
}

}  // namespace nws
