#pragma once

#include <stdint.h>
#include <Preferences.h>

typedef struct {

    float blushLevel = 0; // percent
    int blushLevelPWM = 0;

    int co2 = 30;
    int fan = 100;
    int hum = 30;
    bool isBoop = false;

    int buttonState = -1; // disabled until overridden. 0-4 as only 1 press at a time is supported currently
} globals;

typedef struct {
    int brightnessLevel = 0;
    int currentExpression = 0;

} settings;

extern globals g;
extern settings s;

void updateData();
void saveSettings();
void loadSettings();