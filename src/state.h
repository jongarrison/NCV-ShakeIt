#ifndef STATE_H
#define STATE_H

#include <Arduino.h>

namespace state {

// Enum to represent all possible states
enum State {
    LISTENING,      // Waiting for EMF detection
    EMF_DETECTED,   // EMF detected, monitoring duration
    SPINDOWN_WAIT,  // EMF no longer detected, waiting to activate
    ACTIVATING      // Playing music and activating relays
};

// Initialize the state machine
void init();

// Update the state machine (call this in the main loop)
void update();

// Get the current state
State getCurrentState();

// External events that can trigger state transitions
void onEmfLost();
void onSpindownComplete();
void onActivationComplete();
void onOverrideButtonPressed();

// Force a specific state transition (useful for manual triggers)
void transitionTo(State newState);

// For debugging
const char* getStateName(State state);

} // namespace state

#endif // STATE_H