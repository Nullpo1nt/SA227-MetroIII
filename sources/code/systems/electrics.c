/**
 * DC
 * 28 V
 *
 * External Source
 * 2x Engine Generator-Starter
 * 2x NiCad Batteries
 * - 24 V
 */

#include <stdbool.h>

typedef struct cb_s {
    bool state;
    float limitAmp;
    float amps;
} cb_t;

typedef struct bus_s {
    float volts;
    float amps;
} bus_t;

// gen_t generator_l;
// gen_t generator_r;

// gen_t gpu;

// bat_t battery_l;
// bat_t battery_r;

bus_t battery_bus;

bus_t leftEssential;
bus_t rightEssential;
bus_t nonEssential;

struct component_t {
    struct component_t* next;
};

typedef enum { BATT_OFF, BATT_ON, BATT_RESET } BatSwitch;
BatSwitch leftBattery;
BatSwitch rightBattery;

typedef enum { ANNUNCIATOR_OFF, ANNUNCIATOR_ON, ANNUNCIATOR_BLINK } Annunciator;
Annunciator lBatDisc;
Annunciator rBatDisc;
Annunciator batteryFault;

typedef enum { LEFT_BATT, LEFT_GEN, VOLTMETER_OFF, RIGHT_GEN, RIGHT_BATT, GPU } VoltmeterSwitch;
VoltmeterSwitch voltmeterSwitch;

float leftBatteryTemp;
float leftBatteryVolt;
float rightBatteryTemp;
float rightBatteryVolt;

// Either battery > 120F
bool batteryWarm;

// Either battery > 150F
bool batteryHot;

/**
 * Sweeps indicators from bottom to top evenly over ~5 seconds
 * Warm, Hot should iluminate at correct positions
 */
typedef enum { BTIT_OFF, BTIT_ON } BatTempIndTest;
BatTempIndTest batTempIndTestSwitch;

typedef enum { BTRE_OFF, BTRE_ON } BatTempRangeExtend;
BatTempRangeExtend batTempRangeExtend;

/**
 * Normal range, 100F-190F
 * <120F green
 * 120F-150F yellow
 * >150F red
 *
 * Extend range: (adds 50F to indicator) 50F-100F
 */
float batTempIndicatorLeft;
float batTempIndicatorRight;
/**
 * AC
 * 115 V
 * 26 V
 *
 * 2x Inverters
 */

/**
 * AC
 *
 *
 */