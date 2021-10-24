#ifndef WIFI_CONTROLLER_H_
#define WIFI_CONTROLLER_H_

#include <WiFi.h>

#include "nvs/NVSLoader.h"

class WiFiController {
public:
    static const int8_t WIFI_RETRY = 30;
    static const unsigned long WIFI_RECONNECT_INTERVAL_MS = 30000;

    WiFiController(void);
    virtual ~WiFiController(void);

    virtual bool begin(const char* nvsNapmespace, const char* nvsConfigPath,
                       void (*connectingCallback)(uint8_t retries));
    virtual bool update(void);
    virtual bool isConnected(void) const;
    virtual bool disconnect(void);

    virtual WiFiClient& getClient(void);

protected:
    virtual bool connectWiFi(const char* ssid, const char* passwd,
                             uint8_t retries,
                             void (*connectingCallback)(uint8_t retries));

private:
    WiFiClient client;
    unsigned long prevConnTime;
};

#endif
