#include <M5Atom.h>

#ifdef ENABLE_BLE_KEYBOARD
#include "ble/BLEKeyboardController.h"
const uint8_t SEND_KEY = ' ';
const char DEVICE_NAME[] = "ATOM Keyboard";
const char DEVICE_MANUFACTURER[] = "M5Stack";
const CRGB CRGB_BLE_CONNECTED(0x00, 0xff, 0x00);
const CRGB CRGB_BLE_DISCONNECTED(0xff, 0x00, 0x00);
#endif

#ifdef ENABLE_WIFI
#include "wifi/WiFiController.h"
const CRGB CRGB_WIFI_CONNECTING(0x0, 0xff, 0xff);
const CRGB CRGB_WIFI_CONNECTED(0x00, 0xff, 0x00);
const CRGB CRGB_WIFI_DISCONNECTED(0xff, 0x00, 0x00);
#endif
const CRGB CRGB_STARTED(0x0, 0xff, 0xff);
const CRGB CRGB_BLACK(0x00, 0x00, 0x00);
#include "common.h"

const bool ENABLE_SERIAL = true;
const bool ENABLE_I2C = true;
const bool ENABLE_DISPLAY = true;

#ifdef ENABLE_BLE_KEYBOARD
BLEKeyboardController keyboardController(DEVICE_NAME, DEVICE_MANUFACTURER);
bool isKeyPressed = false;

void doConnect(void) {
    M5.dis.fillpix(CRGB_BLE_CONNECTED);
    SERIAL_PRINTLN("BLE keyboard: Connected");
}

void doUpdate(BLEKeyboardController& controller) {
    if (isKeyPressed) {
        if (M5.Btn.isReleased()) {
            isKeyPressed = false;
            controller.release(SEND_KEY);
            SERIAL_PRINTF_LN("Key '%c': Released", SEND_KEY);
        }
    } else {
        if (M5.Btn.isPressed()) {
            isKeyPressed = true;
            controller.press(SEND_KEY);
            SERIAL_PRINTF_LN("Key '%c': Pressed", SEND_KEY);
        }
    }
}

void doDisconnect(void) {
    M5.dis.fillpix(CRGB_BLE_DISCONNECTED);
    SERIAL_PRINTLN("BLE Keyboard: Disconnected");
}
#endif

#ifdef ENABLE_WIFI
WiFiController wifiController;

void connectingCallback(uint8_t retries) {
    M5.dis.fillpix(retries % 2 == 0 ? CRGB_BLACK : CRGB_WIFI_CONNECTING);
}
#endif

void setup(void) {
    M5.begin(ENABLE_SERIAL, ENABLE_I2C, ENABLE_DISPLAY);
    M5.dis.fillpix(CRGB_STARTED);
#ifdef ENABLE_WIFI
    if (wifiController.begin(connectingCallback)) {
        M5.dis.fillpix(CRGB_WIFI_CONNECTED);
    } else {
        M5.dis.fillpix(CRGB_WIFI_DISCONNECTED);
    }
#endif
#ifdef ENABLE_BLE_KEYBOARD
    keyboardController.begin();
#endif
}

void loop(void) {
    M5.update();
#ifdef ENABLE_WIFI
    wifiController.update();
#endif
#ifdef ENABLE_BLE_KEYBOARD
    keyboardController.update(doConnect, doUpdate, doDisconnect);
#endif
}