#pragma once
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "dataHandler.h"


class BLEHandler {
    public:
    void begin(const char *DeviceName = "ESP32_BLE");
    void notifyAll(const String &payload);
    void setOnFrameReceived(void (*callback)(const String &frame));
    int getConnectedCount() const {return connectedCount;}

    private:
    static constexpr const char *SERVICE_UUID = ""; // check
    static constexpr const char *CHARACTERISTIC_UUID = ""; // check
    int connectedCount = 0;

    BLEServer *pServer = nullptr;
    BLECharacteristic *pCharacteristic = nullptr;

    void (*onFrameReceived)(const String &frame) = nullptr;

    friend class BLEHandlerServerCallbacks;
    friend class BLEHandlerCharCallbacks;

};

extern BLEHandler bleHandler;

String frame;

void handleFrame(const String &frame);