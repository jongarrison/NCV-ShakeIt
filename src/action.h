#ifndef ACTION_H
#define ACTION_H

#include <SimpleTimer.h>
#include <AceButton.h>
using namespace ace_button;

#define PIN_TIMEKNOB A9
#define PIN_SQUELCHKNOB A8
#define PIN_LED_POWER_INDICATOR A1
#define PIN_LED_TRIGGER_INDICATOR A2
#define PIN_RELAY1 A3
#define PIN_RELAY2 A4
#define MP3_RX D7
#define MP3_TX D6
#define MP3_VOLUME_DEFAULT 20
#define PIN_USER_OVERRIDE_BUTTON 0
#define POT_KNOB_MIN_MS 2000
#define POT_KNOB_MAX_MS 30000

typedef void (*EventHandler)(AceButton* button, uint8_t eventType, uint8_t buttonState);

/**
 * The action namespace contains functions that implement the actual
 * behaviors triggered by state transitions in the state machine.
 * 
 * This allows the state machine to focus on
 * state transitions while delegating the implementation details of
 * what happens during those transitions to this module.
 */
namespace action {    
    extern SimpleTimer timer;

    void initHardware(EventHandler eventHandler);
    void doLoop();
    void displayUserEvent();
    void userPlayRandomTrack();
    void userSetVolume();
    bool isEmfDetected();
    int getVolumeStorage();
    void relaysOn();
    void relaysOff();
    int getRelayOnDurationMs();

} // namespace action

#endif // ACTION_H