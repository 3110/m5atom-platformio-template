#include "wifi/WiFiController.h"

#include "common.h"

const char CONFIG_FILE[] = "/settings.json";
const char MAX_SSID_LEN = 32;
const char MAX_SSID_PASSWORD_LEN = 64;
const char KEY_SSID[] = "ssid";
const char KEY_SSID_PASSWORD[] = "password";

WiFiController::WiFiController(void) : client() {
}

WiFiController::~WiFiController(void) {
}

bool WiFiController::begin(const char* nvsNamespace, const char* nvsConfigPath,
                           void (*connectingCallback)(uint8_t retries)) {
    if (nvsNamespace == nullptr) {
        SERIAL_PRINTLN("nvsNamespace is null");
        return false;
    }
    if (nvsConfigPath == nullptr) {
        SERIAL_PRINTLN("nvsConfigPath is null");
        return false;
    }
    NVSLoader loader(nvsNamespace);
    char ssid[MAX_SSID_LEN + 1] = {0};
    char passwd[MAX_SSID_PASSWORD_LEN + 1] = {0};

    if (!loader.begin(nvsConfigPath)) {
        return false;
    }
    if (!loader.get(KEY_SSID, ssid, sizeof(ssid))) {
        SERIAL_PRINTF_LN("Key(%s) is not set", KEY_SSID);
        return false;
    }
    if (!loader.get(KEY_SSID_PASSWORD, passwd, sizeof(passwd))) {
        SERIAL_PRINTF_LN("Key(%s) is not set", KEY_SSID_PASSWORD);
        return false;
    }

    return connectWiFi(ssid, passwd, WIFI_RETRY, connectingCallback);
}

bool WiFiController::update(void) {
    if (WiFi.status() != WL_CONNECTED) {
    }
    return true;
}

bool WiFiController::isConnected(void) const {
    return WiFi.isConnected();
}

bool WiFiController::disconnect(void) {
    return WiFi.disconnect(true);
}

WiFiClient& WiFiController::getClient(void) {
    return this->client;
}

bool WiFiController::connectWiFi(const char* ssid, const char* passwd,
                                 uint8_t retries,
                                 void (*connectingCallback)(uint8_t retries)) {
    SERIAL_PRINTLN();
    SERIAL_PRINT("Connecting to ");
    SERIAL_PRINTLN(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, passwd);
    while (WiFi.status() != WL_CONNECTED && --retries >= 0) {
        if (connectingCallback) {
            connectingCallback(retries);
        }
        SERIAL_PRINT(".");
        delay(1000);
    }
    if (retries < 0) {
        return false;
    }
    SERIAL_PRINTLN();
    SERIAL_PRINTF_LN("WiFi connected: %s", ssid);
    SERIAL_PRINTF_LN("IP address: %s", WiFi.localIP().toString().c_str());
    return true;
}
