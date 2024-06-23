#include "helpers.h"
#include <windows.h>
#include "mcard.h"

// Module name for logging purposes
char module[] = "mcardreader";

// Global variables for state management
bool initialized = false;
MagCard mcard;

extern "C" {
// Exported function for DLL initialization
__declspec(dllexport) void Init() {
    if (!initialized) {
        if (!mcard.initialize()) {
            printWarning("%s, %s: SmartCardReader not initialized\n", __func__, module);
            return;
        }
        initialized = true;
        printInfo("%s, %s: MagCardReader initialized\n", __func__, module);
    }
}

// Exported function for DLL cleanup
__declspec(dllexport) void Exit() {
    printInfo("%s, %s: Exiting MagCardReader\n", __func__, module);

    if (initialized) {
        mcard.~MagCard();
        initialized = false;
    }

    // Create and signal named event
    HANDLE hEvent = CreateEvent(nullptr, TRUE, FALSE, "PluginExitEvent");
    if (hEvent) {
        SetEvent(hEvent);
        CloseHandle(hEvent);
    }
}
}
