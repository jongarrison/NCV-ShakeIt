#include <Arduino.h>
#include "constants.h"
#include "state.h"
#include "action.h"
#include <AceButton.h>
using namespace ace_button;

/**
 * This button handler lives outside of both action and state because it 
 * can initiate actions in both modules.
 */
void handleUserOverrideButton(AceButton* /*button*/, uint8_t eventType, uint8_t /*buttonState*/) {
    Serial.print("Override button event: ");
    Serial.println(eventType);

    if (eventType == AceButton::kEventClicked) {
        Serial.println("kEventClick - Activating relay sequence");
        state::transitionTo(state::ACTIVATING);
        action::displayUserEvent();
    } else if (eventType == AceButton::kEventDoubleClicked) {
        Serial.println("kEventDoubleClicked - Playing random track");
        action::userPlayRandomTrack();
    } else if (eventType == AceButton::kEventLongPressed) {
        Serial.println("kEventLongPressed - Setting volume based on reuse of Timing Knob");
        action::userSetVolume();
    }
}

void setup() {
    action::initHardware(handleUserOverrideButton);
    state::init();

    action::timer.setInterval(500, []() {
        state::update();
    });
}

void loop() {
    action::doLoop();
}

