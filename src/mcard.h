#pragma once

#include <iostream>
#include <windows.h>
#include <chrono>
#include <vector>
#include <nlohmann/json.hpp>

class MagCard {
public:
    MagCard();                   // Constructor
    ~MagCard();                  // Destructor

    bool initialize();           // Method to initialize the keyboard hook

private:
    // Static instance pointer for access in static callback
    static MagCard* instance;

    // Member variables for handling key sequence and timing
    std::vector<char> keySequence;
    std::chrono::steady_clock::time_point lastTimestamp;

    // Two dictionaries to store qr_serial_data and qr_image_data
    nlohmann::json qr_serial_data;
    nlohmann::json qr_image_data;

    // Hook handle
    HHOOK keyboardHook;

    // Hook procedure to be a static member function to use with Windows API
    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

    // Member function to process key events
    void processKeyEvent(char key);

    bool readQrData();
};
