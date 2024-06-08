#include "helpers.h"
#include <windows.h>
#include <thread>
#include <chrono>

void *consoleHandle = nullptr;

void
printColour (int colour, const char *format, ...) {
	va_list args;
	va_start (args, format);

	if (consoleHandle == nullptr) consoleHandle = GetStdHandle (STD_OUTPUT_HANDLE);

	SetConsoleTextAttribute (consoleHandle, colour);
	vprintf (format, args);
	SetConsoleTextAttribute (consoleHandle, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);

	va_end (args);
}

void pressKey(WORD key) {
    INPUT ip = {0};
    ip.type = INPUT_KEYBOARD;
    ip.ki.wVk = key;
    SendInput(1, &ip, sizeof(INPUT));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ip.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &ip, sizeof(INPUT));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
