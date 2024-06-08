#include "helpers.h"
#include <windows.h>
#include "mcard.h"

// Module name for logging purposes
char module[] = "mcardreader";

// Global variables for state management
bool initialized = false;
MagCard mcard;

// Function to initialize the MagCardReader
void InitializeMagCardReader() {
    if (!initialized) {
        if (!mcard.initialize()) {
            printWarning("%s, %s: SmartCardReader not initialized\n", __func__, module);
            return;
        }
        initialized = true;
        printInfo("%s, %s: MagCardReader initialized\n", __func__, module);
    }
}

// Function to clean up and exit the MagCardReader
void ExitMagCardReader() {
    printInfo("%s, %s: Exiting MagCardReader\n", __func__, module);

    if (initialized) {
        mcard.~MagCard();
        initialized = false;
    }

    // Signal a named event to indicate the plugin exit
    HANDLE hEvent = CreateEvent(nullptr, TRUE, FALSE, "PluginExitEvent");
    if (hEvent) {
        SetEvent(hEvent);
        CloseHandle(hEvent);
    }
}

extern "C" {
// Exported function for DLL initialization
__declspec(dllexport) void Init() {
    InitializeMagCardReader();
}

// Exported function for DLL cleanup
__declspec(dllexport) void Exit() {
    ExitMagCardReader();
}
}
