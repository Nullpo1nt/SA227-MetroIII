#include <XPLMPlugin.h>
#include "nws.h"

#include <string.h>

#if IBM
#include <windows.h>
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}
#endif

#ifndef XPLM301
#error This is made to be compiled against the XPLM301 SDK
#endif

PLUGIN_API int XPluginStart(char *outName, char *outSig, char *outDesc) {
    strcpy(outName, "SW4Plugin");
    strcpy(outSig, "sw4.pluginP");
    strcpy(outDesc, "SW4 Aircraft Plugin");

    datarefs::dataRefEditor = XPLMFindPluginBySignature("xplanesdk.examples.DataRefEditor");

    nws::init();
    nws::registerCallbacks();

    return 1;
}

PLUGIN_API int XPluginEnable(void) { return 1; }

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void *inParam) {}

PLUGIN_API void XPluginDisable(void) {}

PLUGIN_API void XPluginStop(void) { nws::unregisterCallbacks(); }
