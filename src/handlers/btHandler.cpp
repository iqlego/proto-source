#include "btHandler.h"

BLEHandler bleHandler;

class BLEHandlerServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) override {
        bleHandler.connectedCount++;
        pServer->getAdvertising()->start();
    }
    void onDisconnect(BLEServer* pServer) override {
        bleHandler.connectedCount--;
        pServer->getAdvertising()->start();
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
    pServer->getAdvertising()->start();

    Serial.println("[BLE] Advertising started");
}

void BLEHandler::notifyAll(const String& payload) {
    if (pCharacteristic == nullptr) return;
    pCharacteristic->setValue(payload.c_str());
    pCharacteristic->notify();
}

void BLEHandler::setOnFrameReceived(void (*callback)(const String& frame)) {
    onFrameReceived = callback;
}

/*
    expected frame from hand example
    L2099
    (header, button state [0 all off, 1 or 2 either button], battery as percentage)
*/
void handleFrame(const String &frame) {
    char frameHeader = frame[0];
    switch(frameHeader) {
        case 'L': // left hand
            g.buttonState = frame[1] - '0';
            return;
        case 'R': // right hand
            g.buttonState = frame[1] - '0' + 2;
            return;
        case 'P': // phone
            return;
        default: // add throw error perchance
            return;
    }
}

// header + helmet battery + l hand battery + r hand battery + current expression index + brightness level
String createFrame(globals) {
    return "H" + g.batP + g.lHandBatP + g.rHandBatP + s.currentExpression + s.brightnessLevel;
}