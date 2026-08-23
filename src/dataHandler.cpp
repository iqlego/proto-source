#include "dataHandler.h"
#include "bitmaps.h"
globals g;
settings s;
Preferences prefs;

void updateData() {
    g.blushLevelPWM = g.blushLevel*40.95; // the blushlevel (percent) converted to digitalWrite scale 0-4095
    s.currentExpression = currentEyeExpression; 
}

void saveSettings() {
    prefs.begin("settings", false);
    prefs.putInt("brightness", s.brightnessLevel);
    prefs.end();
}

void loadSettings() {
    prefs.begin("settings", true);
    s.brightnessLevel = prefs.getInt("brightness");
    prefs.end();
}