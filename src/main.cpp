#include <M5Atom.h>

#include "common.h"

const CRGB CRGB_STARTED(0xff, 0xff, 0xff);
const CRGB CRGB_CONNECTING(0x0, 0xff, 0xff);
const CRGB CRGB_CONNECTED(0x00, 0xff, 0x00);
const CRGB CRGB_DISCONNECTED(0xff, 0x00, 0x00);

#ifdef ENABLE_BLE_KEYBOARD
#include "ble/BLEKeyboardController.h"
const uint8_t SEND_KEY = ' ';
const char DEVICE_NAME[] = "ATOM Keyboard";
const char DEVICE_MANUFACTURER[] = "M5Stack";
#endif

#ifdef ENABLE_WIFI
#include "wifi/WiFiController.h"
#endif

#ifdef ENABLE_ESPNOW
#include "espnow/EspNowManager.h"
#endif

#ifdef ENABLE_MQTT
#include "mqtt/MQTTClient.h"
#endif

const bool ENABLE_SERIAL = true;
const bool ENABLE_I2C = true;
const bool ENABLE_DISPLAY = true;

#ifdef ENABLE_BLE_KEYBOARD
BLEKeyboardController keyboardController(DEVICE_NAME, DEVICE_MANUFACTURER);
bool isKeyPressed = false;

void doConnect(void) {
    M5.dis.fillpix(CRGB_CONNECTED);
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
    M5.dis.fillpix(CRGB_DISCONNECTED);
    SERIAL_PRINTLN("BLE Keyboard: Disconnected");
}
#endif

#if defined(ENABLE_WIFI) || defined(ENABLE_MQTT)
const char WIFI_NVS_NAMESPACE[] = "WiFiController";
const char WIFI_NVS_CONFIG_PATH[] = "/settings.json";
#endif

#ifdef ENABLE_WIFI
WiFiController wifiController(WIFI_NVS_NAMESPACE);

void connectingWiFiCallback(uint8_t retries) {
    M5.dis.fillpix(retries % 2 == 0 ? CRGB::Black : CRGB_CONNECTING);
}
#endif

#ifdef ENABLE_ESPNOW
EspNowManager espNowManager;
const uint8_t MESSAGE[] = "Hello, world!";
const uint32_t MESSAGE_INTERVAL_MS = 5000;

void onDataSent(const uint8_t* addr, esp_now_send_status_t status) {
    SERIAL_PRINT("onDataSent(");
    SERIAL_MAC_ADDRESS_PRINT(addr);
    SERIAL_PRINTF_LN("): %s", status == ESP_NOW_SEND_SUCCESS
                                  ? "Delivery Success"
                                  : "Delivery Fail");
}

void onDataReceived(const uint8_t* addr, const uint8_t* data, int len) {
    SERIAL_PRINT("onDataReceived(");
    SERIAL_MAC_ADDRESS_PRINT(addr);
    SERIAL_PRINTLN("): [");
    SERIAL_DUMP(data, len);
    SERIAL_PRINTLN("]");
}
#endif

#ifdef ENABLE_MQTT
const CRGB CRGB_SUBSCRIBED(0xff, 0xff, 0x00);

const char MQTT_BROKER[] = "192.168.11.82";
const char SAMPLE_TOPIC[] = "m5stack/sample";

const size_t MESSAGE_BUFFER_SIZE = 256;
const size_t MAX_CLIENT_ID_PREFIX_LEN = 10;
const char CLIENT_ID_PREFIX[MAX_CLIENT_ID_PREFIX_LEN + 1] = "atom-mqtt-";
const size_t MAX_CLIENT_ID_LEN = MAX_CLIENT_ID_PREFIX_LEN + ETH_ALEN * 2;
const unsigned long PUBLISH_INTERVAL_MS = 5000;

char CLIENT_ID[MAX_CLIENT_ID_LEN + 1] = {0};
uint8_t MAC_ADDRESS[ETH_ALEN + 1] = {0};
char msg[MESSAGE_BUFFER_SIZE] = {0};
unsigned long lastPublished = 0;
uint32_t publishCount = 0;

bool setMacAddress(uint8_t* buf, size_t size) {
    if (size < ETH_ALEN) {
        return false;
    }
    esp_read_mac(MAC_ADDRESS, ESP_MAC_WIFI_STA);
    return true;
}

void connectingMQTTCallback(uint8_t retries) {
    M5.dis.fillpix(retries % 2 == 0 ? CRGB::Black : CRGB_CONNECTING);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    SERIAL_PRINTF("Message arrived [%s]", topic);
    for (int i = 0; i < length; i++) {
        SERIAL_PRINT((char)payload[i]);
    }
    SERIAL_PRINTLN();
}

MQTTClient mqttClient;
#endif

void setup(void) {
    M5.begin(ENABLE_SERIAL, ENABLE_I2C, ENABLE_DISPLAY);
    M5.dis.fillpix(CRGB_CONNECTING);

#if defined(ENABLE_WIFI) && !defined(ENABLE_MQTT)
    if (wifiController.begin(WIFI_NVS_CONFIG_PATH, connectingWiFiCallback)) {
        M5.dis.fillpix(CRGB_CONNECTED);
    } else {
        M5.dis.fillpix(CRGB_DISCONNECTED);
    }
#endif

#ifdef ENABLE_BLE_KEYBOARD
    keyboardController.begin();
#endif

#ifdef ENABLE_ESPNOW
    espNowManager.begin();
    espNowManager.registerCallback(onDataSent);
    espNowManager.registerCallback(onDataReceived);
    espNowManager.registerPeer(EspNowManager::BROADCAST_ADDRESS);
#endif

#ifdef ENABLE_MQTT
    setMacAddress(MAC_ADDRESS, sizeof(MAC_ADDRESS));
    SERIAL_PRINT("Mac Address:");
    SERIAL_MAC_ADDRESS_PRINTLN(MAC_ADDRESS);

    snprintf(CLIENT_ID, sizeof(CLIENT_ID), "%s%02x%02x%02x%02x%02x%02x",
             CLIENT_ID_PREFIX, MAC_ADDRESS[0], MAC_ADDRESS[1], MAC_ADDRESS[2],
             MAC_ADDRESS[3], MAC_ADDRESS[4], MAC_ADDRESS[5]);
    mqttClient.setId(CLIENT_ID);
    if (!mqttClient.begin(WIFI_NVS_NAMESPACE, WIFI_NVS_CONFIG_PATH,
                          connectingMQTTCallback)) {
        M5.dis.fillpix(CRGB_DISCONNECTED);
        while (true) {
        }
    }
    mqttClient.setup(mqttCallback, MQTT_BROKER);
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

#ifdef ENABLE_ESPNOW
    espNowManager.send(EspNowManager::BROADCAST_ADDRESS, MESSAGE,
                       sizeof(MESSAGE));
    delay(MESSAGE_INTERVAL_MS);
#endif

#ifdef ENABLE_MQTT
    if (!mqttClient.connected()) {
        if (mqttClient.reconnect()) {
            M5.dis.fillpix(CRGB_CONNECTED);
            if (mqttClient.subscribe(SAMPLE_TOPIC, 0)) {
                M5.dis.fillpix(CRGB_SUBSCRIBED);
                SERIAL_PRINTF_LN("Subscribe: %s", SAMPLE_TOPIC);
            } else {
                SERIAL_PRINTF_LN("Failed to Subscribe: %s", SAMPLE_TOPIC);
            }
        } else {
            M5.dis.fillpix(CRGB_DISCONNECTED);
        }
    }
    mqttClient.update();
    unsigned long now = millis();
    if (now - lastPublished > PUBLISH_INTERVAL_MS) {
        lastPublished = now;
        snprintf(msg, MESSAGE_BUFFER_SIZE, "Hello world #%u from %s.",
                 ++publishCount, CLIENT_ID);
        mqttClient.publish(SAMPLE_TOPIC, msg);
    }
#endif
}