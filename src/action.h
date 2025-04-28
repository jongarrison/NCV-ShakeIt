#ifndef ACTION_H
#define ACTION_H

#include "constants.h"
#include <SimpleTimer.h>
#include <AceButton.h>
using namespace ace_button;

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