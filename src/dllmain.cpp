#include "helpers.h"
#include <windows.h>
#include "mcard.h"

char module[] = "mcardreader";

bool initialized = false;
MagCard mcard;

extern "C" {
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