#pragma once
#include <Arduino.h>
#include <BLE2902.h>
#include <BLEAdvertisedDevice.h>
#include <BLEClient.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include "dataHandler.h"
#include "tools.h"

static BLEUUID HM10_SERVICE_UUID("0000FFE0-0000-1000-8000-00805F9B34FB");
static BLEUUID HM10_CHAR_UUID("0000FFE1-0000-1000-8000-00805F9B34FB");

extern struct HandLink hands[2];
void scanAndConnectHands();

class BLEHandler {
    public:
    void begin(const char* DeviceName = "ESP32_BLE");
    void notifyAll(const String& payload);
    void setOnFrameReceived(void (*callback)(const String& frame));
    int getConnectedCount() const { return connectedCount; }

    private:
    static constexpr const char* SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
    static constexpr const char* CHARACTERISTIC_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8";
    int connectedCount = 0;

    BLEServer* pServer = nullptr;
    BLECharacteristic* pCharacteristic = nullptr;

    void (*onFrameReceived)(const String& frame) = nullptr;

    friend class BLEHandlerServerCallbacks;
    friend class BLEHandlerCharCallbacks;
};

extern BLEHandler bleHandler;

void handleFrame(const String& frame);

void transmitBtPeriodic();
void scanHandsPeriodic();