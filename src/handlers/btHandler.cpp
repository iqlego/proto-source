#include "btHandler.h"

BLEHandler bleHandler;

class BLEHandlerServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) override {
        bleHandler.connectedCount++;
        pServer->getAdvertising()->start();
    }
    void onDisconnect(BLEServer* pServer) override {
        if (bleHandler.connectedCount > 0) bleHandler.connectedCount--;
        pServer->getAdvertising()->start();
        if (bleHandler.connectedCount <= 0) {
            g.buttonState = -1;
        }
    }
};

class BLEHandlerCharCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pChar) override {
        std::string value = pChar->getValue();
        if (value.length() > 0) {
            String frame = String(value.c_str());
            if (bleHandler.onFrameReceived) {
                bleHandler.onFrameReceived(frame);
            } else {
                Serial.print("[BLE] Frame (no handler set): ");
                Serial.println(frame);
            }
        }
    }
};

void BLEHandler::begin(const char* deviceName) {
    BLEDevice::init(deviceName);

    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new BLEHandlerServerCallbacks());

    BLEService* pService = pServer->createService(SERVICE_UUID);

    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ |
                                 BLECharacteristic::PROPERTY_WRITE |
                                 BLECharacteristic::PROPERTY_NOTIFY);

    pCharacteristic->setCallbacks(new BLEHandlerCharCallbacks());
    pCharacteristic->addDescriptor(new BLE2902());

    pService->start();

    BLEAdvertising* pAdvertising = pServer->getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->start();

    Serial.println("[BLE] Advertising started");
}

void BLEHandler::notifyAll(const String& payload) {
    if (pCharacteristic == nullptr)
        return;
    pCharacteristic->setValue(payload.c_str());
    pCharacteristic->notify();
}

void BLEHandler::setOnFrameReceived(void (*callback)(const String& frame)) {
    onFrameReceived = callback;
}

struct HandLink {
    const char* name;
    BLEClient* client = nullptr;
    bool connected = false;
    HandLink(const char* n) : name(n) {}
};

class HandClientCallbacks : public BLEClientCallbacks {
    public:
    HandLink* hand;
    HandClientCallbacks(HandLink* h) : hand(h) {}
    void onConnect(BLEClient* pClient) override {}
    void onDisconnect(BLEClient* pClient) override {
        hand->connected = false;
    }
};

HandLink hands[2] = {
    {"HAND_L"},
    {"HAND_R"}};

static void notifyCallback(BLERemoteCharacteristic* pChar, uint8_t* data, size_t length, bool isNotify) {
    String frame((char*)data, length);
    handleFrame(frame);
}

bool connectToHand(HandLink& hand, BLEAdvertisedDevice& device) {
    hand.client = BLEDevice::createClient();
    hand.client->setClientCallbacks(new HandClientCallbacks(&hand));
    if (!hand.client->connect(&device)) return false;

    BLERemoteService* pService = hand.client->getService(HM10_SERVICE_UUID);
    if (pService == nullptr) {
        hand.client->disconnect();
        return false;
    }

    BLERemoteCharacteristic* pChar = pService->getCharacteristic(HM10_CHAR_UUID);
    if (pChar == nullptr) {
        hand.client->disconnect();
        return false;
    }

    if (pChar->canNotify()) {
        pChar->registerForNotify(notifyCallback);
    }

    hand.connected = true;
    return true;
}

static void onScanComplete(BLEScanResults results) {
    for (int i = 0; i < results.getCount(); i++) {
        BLEAdvertisedDevice device = results.getDevice(i);
        for (auto& hand : hands) {
            if (!hand.connected && device.getName() == hand.name) {
                connectToHand(hand, device);
            }
        }
    }
    BLEDevice::getScan()->clearResults();
}

void scanAndConnectHands() {
    BLEScan* pScan = BLEDevice::getScan();
    if (pScan->isScanning()) return;
    pScan->start(3, onScanComplete, false);
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

// inbound: BT connection status

// outbound: header + helmet battery + l hand battery + r hand battery + current expression index + brightness level
String createFrame() {
    return "H" + String(g.batP) + String(g.lHandBatP) + String(g.rHandBatP) + String(s.currentExpression) + String(s.brightnessLevel);
}

unsigned long lastFrameSendTime = 0;
unsigned long lastScanTime = 0;

void transmitBtPeriodic() {
    if (isTimerFinished(lastFrameSendTime, 500)) {
        bleHandler.notifyAll(createFrame());
        lastFrameSendTime = millis();
    }
}

void scanHandsPeriodic() {
    if (isTimerFinished(lastScanTime, 5000)) {
        scanAndConnectHands();
        lastScanTime = millis();
    }
}