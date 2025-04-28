#include "Arduino.h"
#include "state.h"
#include "action.h"
#include "ncv.h"
#include "dfplay.h"

namespace state {
    // Current state of the system
    static State currentState = LISTENING;

    // Timestamp when EMF was first detected
    static unsigned long emfFirstDetectedTime = 0;

    // Makes sure that the relays get run even if multiple EMF detection cycles occur
    // Imagine the scenario of a solid triggering, but during the spindown wait a short EMF
    // detection occurs, this would clear the spindown wait instead of restarting it
    static bool isRelayRunNeeded = false;

    // Timer IDs for the different timers we'll use
    static int spindownTimerId = -1;
    static int musicTimerId = -1;
    static int relayStartTimerId = -1;
    static int relayStopTimerId = -1;
    static int spindownIndicatorTimerId = -1;

    // Helper function to cancel all timers
    void cancelAllTimers() {
        if (spindownTimerId != -1) {
            action::timer.deleteTimer(spindownTimerId);
            spindownTimerId = -1;
        }
        if (musicTimerId != -1) {
            action::timer.deleteTimer(musicTimerId);
            musicTimerId = -1;
        }
        if (relayStartTimerId != -1) {
            action::timer.deleteTimer(relayStartTimerId);
            relayStartTimerId = -1;
        }
        if (relayStopTimerId != -1) {
            action::timer.deleteTimer(relayStopTimerId);
            relayStopTimerId = -1;
        }
        if (spindownIndicatorTimerId != -1) {
            action::timer.deleteTimer(spindownIndicatorTimerId);
            spindownIndicatorTimerId = -1;
        }
    }

    void refreshDisplay() {
       // Entry actions for the new state
       switch (currentState) {
        case LISTENING:
            digitalWrite(PIN_LED_POWER_INDICATOR, HIGH);
            digitalWrite(PIN_LED_TRIGGER_INDICATOR, LOW);
            break;
            
        case EMF_DETECTED:
            digitalWrite(PIN_LED_POWER_INDICATOR, LOW);
            digitalWrite(PIN_LED_TRIGGER_INDICATOR, HIGH);
            break;
            
        case SPINDOWN_WAIT:
            digitalWrite(PIN_LED_POWER_INDICATOR, HIGH);
            digitalWrite(PIN_LED_TRIGGER_INDICATOR, LOW);
            break;
            
        case ACTIVATING:            
            digitalWrite(PIN_LED_POWER_INDICATOR, LOW);
            digitalWrite(PIN_LED_TRIGGER_INDICATOR, HIGH);
            break;
    }        
    }

    // State transition function
    void transitionTo(State newState) {
        // Don't transition if we're already in that state
        if (newState == currentState) return;
        
        Serial.print("State transition: ");
        Serial.print(getStateName(currentState));
        Serial.print(" -> ");
        Serial.println(getStateName(newState));
        
        // Exit actions for the current state
        switch (currentState) {
            case EMF_DETECTED:
                // Reset EMF detection time
                emfFirstDetectedTime = 0;
                break;
                
            case SPINDOWN_WAIT:
                cancelAllTimers();
                break;
                
            case ACTIVATING:
                cancelAllTimers();
                action::relaysOff();
                dfplay::stop();                
                break;
                
            default:
                break;
        }
        
        // Update the current state
        currentState = newState;
        
        // Entry actions for the new state
        // LED state will be updated after the switch statement
        switch (newState) {
            case LISTENING:
                //All the work for LISTENING is done in the update function
                break;
                
            case EMF_DETECTED:
                // Record when EMF was first detected
                if (emfFirstDetectedTime == 0) {
                    emfFirstDetectedTime = millis();
                }                
                break;
                
            case SPINDOWN_WAIT:
                // Start the spindown timer
                spindownTimerId = action::timer.setTimeout(STATE_SPINDOWN_WAIT_TIME_MS, []() {
                    onSpindownComplete();
                });

                spindownIndicatorTimerId = action::timer.setInterval(1000, []() {
                    // Blink the LED to indicate spindown wait
                    static bool greenLedState = LOW;
                    greenLedState = !greenLedState;
                    digitalWrite(PIN_LED_TRIGGER_INDICATOR, greenLedState);
                });
                break;
                
            case ACTIVATING:
                // Start playing music
                dfplay::setVolume(action::getVolumeStorage());
                dfplay::playRandomTrack();
                
                // Set timer to stop music after the specified time
                musicTimerId = action::timer.setTimeout(STATE_MUSIC_PLAY_TIME_MS, []() {
                    dfplay::stop();
                });
            
                // Set timer to activate relays after music stops
                relayStartTimerId = action::timer.setTimeout(STATE_MUSIC_PLAY_TIME_MS + 500, []() {
                    action::relaysOn();
                    
                    // After relay sequence completes, transition back to LISTENING
                    // This is handled by the activateRelaySequence function which will
                    // set a timer to turn off the relays and set isEmfScanningOn to true
                });

                relayStopTimerId = action::timer.setTimeout(STATE_MUSIC_PLAY_TIME_MS + 500 + action::getRelayOnDurationMs(), []() {
                    isRelayRunNeeded = false; // the relay run cycle completed
                    onActivationComplete();
                });                
                break;
        }
        refreshDisplay();
    }

    // Initialize the state machine
    void init() {
        currentState = LISTENING;
        emfFirstDetectedTime = 0;
        cancelAllTimers();
        
        // Set initial state
        transitionTo(LISTENING);
    }

    // Update the state machine based on current conditions
    void update() {
        // Check for EMF detection
        bool wasEmfDetected = action::isEmfDetected();

        switch (currentState) {
            case LISTENING:
                // If EMF is detected, transition to EMF_DETECTED
                if (wasEmfDetected) {
                    transitionTo(EMF_DETECTED);
                }
                break;
                
            case EMF_DETECTED:
                // If EMF is no longer detected and minimum time has passed
                if (!wasEmfDetected && (millis() - emfFirstDetectedTime > STATE_MINIMUM_EMF_DETECTION_TIME_MS)) {
                    //A solid triggering has occurred
                    isRelayRunNeeded = true; 
                    transitionTo(SPINDOWN_WAIT);
                } else if (!wasEmfDetected) {
                    if (isRelayRunNeeded) {
                        // We had a solid triggering earlier, if not now, so we return to the spindown wait
                        transitionTo(SPINDOWN_WAIT);
                    } else {
                        // We did not have a solid triggering, so we return to listening
                        transitionTo(LISTENING);
                    }
                }
                break;
                
            case SPINDOWN_WAIT:
                // If EMF is detected again, go back to EMF_DETECTED
                if (wasEmfDetected) {
                    transitionTo(EMF_DETECTED);
                }
                break;
                
            case ACTIVATING:
                // If EMF is detected during activation, immediate go back to EMF_DETECTED
                if (wasEmfDetected) {
                    transitionTo(EMF_DETECTED);
                }
                break;
        }
    }

    void onEmfLost() {
        // Only process this event if we're in EMF_DETECTED state and minimum time has passed
        if (currentState == EMF_DETECTED && 
            millis() - emfFirstDetectedTime > STATE_MINIMUM_EMF_DETECTION_TIME_MS) {
            transitionTo(SPINDOWN_WAIT);
        }
    }

    void onSpindownComplete() {
        // Only process this event if we're in SPINDOWN_WAIT state
        if (currentState == SPINDOWN_WAIT) {
            transitionTo(ACTIVATING);
        }
    }

    void onActivationComplete() {
        // Only process this event if we're in ACTIVATING state
        if (currentState == ACTIVATING) {
            transitionTo(LISTENING);
        }
    }

    void onOverrideButtonPressed() {
        // Manual override to start the activation sequence
        transitionTo(ACTIVATING);
    }

    // Get the current state
    State getCurrentState() {
        return currentState;
    }

    // Get a string representation of the state name (for debugging)
    const char* getStateName(State state) {
        switch (state) {
            case LISTENING: return "LISTENING";
            case EMF_DETECTED: return "EMF_DETECTED";
            case SPINDOWN_WAIT: return "SPINDOWN_WAIT";
            case ACTIVATING: return "ACTIVATING";
            default: return "UNKNOWN";
        }
    }

} // namespace state
