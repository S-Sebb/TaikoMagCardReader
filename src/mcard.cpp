#include "mcard.h"
#include "helpers.h"
#include "constants.h"
#include <fstream>

extern char module[];

// Static instance pointer initialization
MagCard* MagCard::instance = nullptr;

// Constructor
MagCard::MagCard() : keyboardHook(nullptr) {
    // Initialize static instance pointer to this instance
    instance = this;
}

// Destructor
MagCard::~MagCard() {
    if (keyboardHook) {
        UnhookWindowsHookEx(keyboardHook);
    }
}

// Initialize method
bool MagCard::initialize() {
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
    if (keyboardHook == NULL) {
        printError("%s, %s: Failed to install keyboard hook\n", __func__, module);
        return false;
    }

    if (!readQrData()) {
        return false;
    }

    return true;
}

// Static hook procedure
LRESULT CALLBACK MagCard::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT *kbdStruct = (KBDLLHOOKSTRUCT*)lParam;
        char key = (char)kbdStruct->vkCode;

        // Forward the key event to the instance for processing
        if (key >= '0' && key <= '9' && instance) {
            instance->processKeyEvent(key);
        }
    }

    // Call the next hook in the chain
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// Process key event method
void MagCard::processKeyEvent(char key) {
    auto now = std::chrono::steady_clock::now();

    if (!keySequence.empty()) {
        // Calculate the time difference between the current and last key press
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTimestamp);

        // Check if the time difference exceeds the threshold
        if (duration.count() > time_threshold_ms) {
            // Clear the sequence if the time threshold is exceeded
            keySequence.clear();
        }
    }

    // Add the key press to the sequence and update the last timestamp
    keySequence.push_back(key);
    lastTimestamp = now;

    // Check if we have enough key presses to evaluate
    if (keySequence.size() >= card_number_length) {
        // Convert the key sequence to a string
        std::string cardNumber(keySequence.begin(), keySequence.end());

        // Clear the key sequence
        keySequence.clear();

        // Check cardNumber is in qr_serial_data, or qr_image_data or else, assume as Access Code
        if (qr_serial_data.contains(cardNumber)) {
            // Convert the QR data to a string
            std::string qr_serial = qr_serial_data[cardNumber].dump();
            // Strip the quotes from the string
            qr_serial = qr_serial.substr(1, qr_serial.size() - 2);

            printInfo("Card Type: %s\n", "qr_serial");
            printInfo("QR Serial Data: %s\n", qr_serial.c_str());

            // Write qr_serial to qr.dat
            std::ofstream fp("qr.dat");
            if (fp.is_open()) {
                fp << qr_serial;
                fp.close();
            } else {
                printError("%s, %s: Failed to open qr.dat\n", __func__, module);
            }

            // Press F4 key
            pressKey(0x73);
        } else if (qr_image_data.contains(cardNumber)) {
            // Convert the QR data to a string
            std::string qr_image = qr_image_data[cardNumber].dump();
            // Strip the quotes from the string
            qr_image = qr_image.substr(1, qr_image.size() - 2);

            printInfo("Card Type: %s\n", "qr_image");
            printInfo("QR Image Data: %s\n", qr_image.c_str());

            // Press F6 key
            pressKey(0x75);
        } else {
            // Check if cardNumber begins with 20000
            if (cardNumber.substr(0, 5) != "20000") {
                printWarning("Invalid card number: %s\n", cardNumber.c_str());
                return;
            }

            printInfo("Card Type: %s\n", "magnetic");
            printInfo("Access Code: %s\n", cardNumber.c_str());

            // Write the key sequence to cards.dat
            std::ofstream fp("cards.dat");
            if (fp.is_open()) {
                fp << cardNumber;
                fp.close();
            } else {
                printError("%s, %s: Failed to open cards.dat\n", __func__, module);
            }

            // Press F3 key
            pressKey(0x72);
        }
    }
}

bool MagCard::readQrData() {
    // Read QR data from qr_data.json
    nlohmann::json qrData;
    std::ifstream fp("qr_data.json");
    if (fp.is_open()) {
        try {
            fp >> qrData;
        } catch (nlohmann::json::parse_error& e) {
            fp.close();
            printError("%s, %s: Failed to parse qr_data.json: %s\n", __func__, module, e.what());
            return false;
        }
        fp.close();
    } else {
        // Try plugins/qr_data.json
        fp.open("plugins/qr_data.json");
        if (fp.is_open()) {
            try {
                fp >> qrData;
            } catch (nlohmann::json::parse_error& e) {
                fp.close();
                printError("%s, %s: Failed to parse plugins/qr_data.json: %s\n", __func__, module, e.what());
                return false;
            }
            fp.close();
        } else {
            printError("%s, %s: Failed to open qr_data.json\n", __func__, module);
            return false;
        }
    }

    // Check if qr_serial_data and qr_image_data are present
    if (qrData.contains("qr_serial_data")) {
        qr_serial_data = qrData["qr_serial_data"];
    } else {
        printError("%s, %s: qr_serial_data not found in qr_data.json\n", __func__, module);
        return false;
    }

    if (qrData.contains("qr_image_data")) {
        qr_image_data = qrData["qr_image_data"];
    } else {
        printError("%s, %s: qr_image_data not found in qr_data.json\n", __func__, module);
        return false;
    }

    return true;
}
