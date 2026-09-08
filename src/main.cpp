// TODO: implement telemetry reading
//       implement bluetooth handler

#include <EEPROM.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <Wire.h>

#include "bitmaps.h"
#include "handlers/btHandler.h"
#include "handlers/dataHandler.h"
#include "handlers/errorHandler.h"
#include "handlers/inputHandler.h"
#include "matrix.h"
#include "screenTypes.h"
#include "screens.h"
#include "tools.h"

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

void setup() {
    /// initIfCrash();
    const char* crashMsg = initIfCrash();
    if (crashMsg) {
        currentEyeExpression = 4;
        isBlinkEnabled = false;
    }
    popup(crashMsg, true);
    setBreadcrumb("boot");

    loadSettings();

    initButtons();

    initMatrices();
    initLeds();

    bleHandler.setOnFrameReceived(handleFrame);
    bleHandler.begin();

    renderFace();
    updateLeds();
    Serial.begin(115200);

    u8g2.begin();

    screenSwitch(Screen::HOME);
}

void loop() {
    pollInputs();

    screenSwitch(g_currentScreen);
    updateTalking();

    updateLeds();
    updateData();
    Serial.print("blush ");
    Serial.print(g.blushLevel);
    Serial.print(" pwm ");
    Serial.println(g.blushLevelPWM);

    bool blinkingNow = isBlinking();
    if (blinkingNow || wasBlinking) {
        renderFace();
    }
    wasBlinking = blinkingNow;

    delay(10);
}
