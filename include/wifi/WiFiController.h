#ifndef WIFI_CONTROLLER_H_
#define WIFI_CONTROLLER_H_

#include <WiFi.h>

#include "nvs/NVSLoader.h"

class WiFiController {
public:
    static const int8_t WIFI_RETRY = 30;
    static const unsigned long WIFI_RECONNECT_INTERVAL_MS = 30000;

    WiFiController(const char* nvsNamespace);
    virtual ~WiFiController(void);

    virtual bool begin(const char* nvsConfigPath,
                       void (*connectingCallback)(uint8_t retries));
    virtual bool update(void);
    virtual bool isConnected(void) const;
    virtual bool disconnect(void);

    virtual bool getPreference(const char* key, char* buf, size_t size);

    virtual WiFiClient& getClient(void);

protected:
    virtual bool connectWiFi(const char* ssid, const char* passwd,
                             uint8_t retries,
                             void (*connectingCallback)(uint8_t retries));

private:
    WiFiClient client;
    NVSLoader nvsLoader;
    unsigned long prevConnTime;
};

#endif
