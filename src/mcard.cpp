#include "mcard.h"
#include "helpers.h"
#include "constants.h"
#include <fstream>

extern char module[];

// Static instance pointer initialization
MagCard* MagCard::instance = nullptr;

// Constructor
MagCard::MagCard() : keyboardHook(nullptr) {
    instance = this; // Assign this instance to the static pointer
}

// Destructor
MagCard::~MagCard() {
    if (keyboardHook) {
        UnhookWindowsHookEx(keyboardHook); // Remove the keyboard hook
    }
}

// Initialize the MagCard
bool MagCard::initialize() {
    // Set up the low-level keyboard hook
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, nullptr, 0);
    if (keyboardHook == nullptr) {
        printError("%s, %s: Failed to install keyboard hook\n", __func__, module);
        return false;
    }

    // Read QR data from files
    if (!readQrData()) {
        return false;
    }

    return true;
}

// Static callback for low-level keyboard events
LRESULT CALLBACK MagCard::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN) {
        auto* kbdStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        char key = static_cast<char>(kbdStruct->vkCode);

        // Process numeric keys if the instance is valid
        if (key >= '0' && key <= '9' && instance) {
            instance->processKeyEvent(key);
        }
    }

    // Pass the event to the next hook in the chain
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

// Process individual key events
void MagCard::processKeyEvent(char key) {
    auto now = std::chrono::steady_clock::now();

    // Clear the sequence if the time threshold is exceeded
    if (!keySequence.empty()) {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTimestamp);
        if (duration.count() > time_threshold_ms) {
            keySequence.clear();
        }
    }

    // Add the key to the sequence and update the timestamp
    keySequence.push_back(key);
    lastTimestamp = now;

    // Process the card number if the sequence is complete
    if (keySequence.size() >= card_number_length) {
        std::string cardNumber(keySequence.begin(), keySequence.end());
        keySequence.clear(); // Reset the sequence for the next card

        // Process the card number based on its type
        if (qr_serial_data.contains(cardNumber)) {
            processQrSerial(cardNumber);
        } else if (qr_image_data.contains(cardNumber)) {
            processQrImage(cardNumber);
        } else {
            processAccessCode(cardNumber);
        }
    }
}

// Read QR data from the file
bool MagCard::readQrData() {
    // Attempt to read from qr_data.json
    nlohmann::json qrData;
    std::ifstream fp("qr_data.json");
    if (!fp.is_open()) {
        // Try reading from the plugins directory if the file is not found
        fp.open("plugins/qr_data.json");
    }

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
        printError("%s, %s: Failed to open qr_data.json\n", __func__, module);
        return false;
    }

    // Extract QR serial and image data
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

// Process QR serial card type
void MagCard::processQrSerial(const std::string& cardNumber) {
    std::string qr_serial = qr_serial_data[cardNumber].dump();
    qr_serial = qr_serial.substr(1, qr_serial.size() - 2); // Strip quotes

    printInfo("Card Type: %s\n", "qr_serial");
    printInfo("QR Serial Data: %s\n", qr_serial.c_str());

    // Write the QR serial data to a file
    std::ofstream fp("qr.dat");
    if (fp.is_open()) {
        fp << qr_serial;
        fp.close();
    } else {
        printError("%s, %s: Failed to open qr.dat\n", __func__, module);
    }

    // Simulate pressing the F4 key
    pressKey(0x73);
}

// Process QR image card type
void MagCard::processQrImage(const std::string& cardNumber) {
    std::string qr_image = qr_image_data[cardNumber].dump();
    qr_image = qr_image.substr(1, qr_image.size() - 2); // Strip quotes

    printInfo("Card Type: %s\n", "qr_image");
    printInfo("QR Image Data: %s\n", qr_image.c_str());

    // Simulate pressing the F6 key
    pressKey(0x75);
}

// Process magnetic access code card type
void MagCard::processAccessCode(const std::string& cardNumber) {
    // Validate the card number prefix
    if (cardNumber.substr(0, 5) != "20000") {
        printWarning("Invalid card number: %s\n", cardNumber.c_str());
        return;
    }

    printInfo("Card Type: %s\n", "magnetic");
    printInfo("Access Code: %s\n", cardNumber.c_str());

    // Write the access code to a file
    std::ofstream fp("cards.dat");
    if (fp.is_open()) {
        fp << cardNumber;
        fp.close();
    } else {
        printError("%s, %s: Failed to open cards.dat\n", __func__, module);
    }

    // Simulate pressing the F3 key
    pressKey(0x72);
}
