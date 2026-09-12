#include "btHandler.h"

#include "NimBLEDevice.h"

NimBLECharacteristic* pCharacteristic = nullptr;
NimBLEServer* pServer = nullptr;
NimBLEService* pService = nullptr;

NimBLEClient* clientL = nullptr;
NimBLEClient* clientR = nullptr;

void initBLE() {
    NimBLEDevice::init("Toast");

    pServer = NimBLEDevice::createServer();
    pService = pServer->createService("ABCD");
    pCharacteristic = pService->createCharacteristic("1234", NIMBLE_PROPERTY::NOTIFY);

    pService->start();

    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID("ABCD");
    pAdvertising->setName("Toast");
    pAdvertising->start();
}

unsigned long lastNotify = 0;

void notifyAllPeriodic() {
    if (isTimerFinished(lastNotify, 500)) {
        pCharacteristic->setValue(createFrame());
        pCharacteristic->notify();
        lastNotify = millis();
    }
}

/*
    expected frame from hand example
    L2099
    (header, button state [0 all off, 1 or 2 either button], battery as percentage)
*/

int lastLeftButton = 0;
int lastRightButton = 0;

void setButtonIndex(char frameHeader, int buttonValue) {
    bool isLeftHand = (frameHeader == 'L');
    if (isLeftHand && buttonValue != lastLeftButton) {
        lastLeftButton = buttonValue;
        g.buttonState = lastLeftButton;
    } else if (!isLeftHand && buttonValue != lastRightButton) {
        lastRightButton = buttonValue;
        g.buttonState = lastRightButton + 2;
    }
}

void handleFrame(const String& frame) {
    if (frame.length() < 5) return;
    char frameHeader = frame[0];
    char buttonValueChar = frame[1];
    int buttonValue = buttonValueChar - '0';

    if (frameHeader == 'L' || frameHeader == 'R') {
        setButtonIndex(frameHeader, buttonValue);
    } else if (frameHeader == 'P') {
        g.fan = frame.substring(2, 5).toInt();
    }
}

String createFrame() {
    return "H" + String(g.batP) + String(g.lHandBatP) + String(g.rHandBatP) + String(s.currentExpression) + String(s.brightnessLevel);
}

void notifyCallback(NimBLERemoteCharacteristic* pChar, uint8_t* data, size_t length, bool isNotify) {
    String frame((char*)data, length);
    handleFrame(frame);
}

class HandDisconnectCallback : public NimBLEClientCallbacks {
    NimBLEClient** slot;

    public:
    HandDisconnectCallback(NimBLEClient** s) : slot(s) {}
    virtual void onDisconnect(NimBLEClient* pClient, int reason) override {
        *slot = nullptr;
    }
};

bool connectAndSubscribe(const NimBLEAdvertisedDevice* device, NimBLEClient*& outClient, void (*cb)(NimBLERemoteCharacteristic*, uint8_t*, size_t, bool)) {
    NimBLEClient* client = NimBLEDevice::createClient();

    if (!client->connect(device)) {
        NimBLEDevice::deleteClient(client);
        return false;
    }

    NimBLERemoteService* pSvc = client->getService(HM10_SERVICE_UUID);
    if (!pSvc) {
        client->disconnect();
        return false;
    }

    NimBLERemoteCharacteristic* pChar = pSvc->getCharacteristic(HM10_CHAR_UUID);
    if (!pChar || !pChar->canNotify()) {
        client->disconnect();
        return false;
    }

    if (!pChar->subscribe(true, cb)) {
        client->disconnect();
        return false;
    }

    client->setClientCallbacks(new HandDisconnectCallback(&outClient));
    outClient = client;
    return true;
}

void onScanComplete(NimBLEScanResults results) {
    for (int i = 0; i < results.getCount(); i++) {
        const NimBLEAdvertisedDevice* device = results.getDevice(i);

        if (clientL == nullptr && device->getName() == "TOASTLPAW") {
            connectAndSubscribe(device, clientL, notifyCallback);
        }
        if (clientR == nullptr && device->getName() == "TOASTRPAW") {
            connectAndSubscribe(device, clientR, notifyCallback);
        }
    }
}

unsigned long lastScanTime = 0;

void scanAndConnectHands() {
    NimBLEScan* pScan = NimBLEDevice::getScan();
    if (pScan->isScanning()) return;
    pScan->start(3, onScanComplete, false);
}

void scanHandsPeriodic() {
    if (clientL != nullptr && clientR != nullptr) return;
    if (isTimerFinished(lastScanTime, 5000)) {
        scanAndConnectHands();
        lastScanTime = millis();
    }
}