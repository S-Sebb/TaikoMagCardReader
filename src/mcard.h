#pragma once

#include <iostream>
#include <windows.h>
#include <chrono>
#include <vector>
#include <nlohmann/json.hpp>

class MagCard {
public:
    MagCard();                    // Constructor
    ~MagCard();                   // Destructor

    bool initialize();            // Method to initialize the keyboard hook

private:
    static MagCard* instance;     // Static instance pointer for access in static callback

    std::vector<char> keySequence; // Vector to store the key sequence
    std::chrono::steady_clock::time_point lastTimestamp; // Timestamp of the last key press

    nlohmann::json qr_serial_data; // JSON data for QR serial
    nlohmann::json qr_image_data;  // JSON data for QR image

    HHOOK keyboardHook;            // Hook handle for keyboard events

    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam); // Static hook procedure
    void processKeyEvent(char key); // Method to process key events
    bool readQrData();              // Method to read QR data from a file
    void processQrSerial(const std::string& cardNumber); // Method to process QR serial data
    void processQrImage(const std::string& cardNumber);  // Method to process QR image data
    void processAccessCode(const std::string& cardNumber); // Method to process access codes
};
