#pragma once
#include "NimBLEDevice.h"
#include "handlers/dataHandler.h"
#include "string.h"
#include "tools.h"

void initBLE();
String createFrame();
extern NimBLECharacteristic* pCharacteristic;
extern NimBLEServer* pServer;
extern NimBLEService* pService;

static NimBLEUUID HM10_SERVICE_UUID("0000FFE0-0000-1000-8000-00805F9B34FB");
static NimBLEUUID HM10_CHAR_UUID("0000FFE1-0000-1000-8000-00805F9B34FB");

extern NimBLEClient* clientL;
extern NimBLEClient* clientR;

void scanHandsPeriodic();
void notifyAllPeriodic();