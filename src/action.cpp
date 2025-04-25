#include <Arduino.h>
#include "action.h"
#include "ncv.h"
#include "dfplay.h"
#include "potknob.h"
#include <FlashStorage.h>

namespace action {
    void setVolumeStorage(int volume);
    int getVolumeStorage();
    
    SimpleTimer timer;
    AceButton overrideButton(static_cast<double>(PIN_USER_OVERRIDE_BUTTON));
    PotKnob relayOnTimeKnob(PIN_TIMEKNOB, 2, 1023, POT_KNOB_MAX_MS, POT_KNOB_MIN_MS); //Notice the reversed knob values, knobs installed upside down
    PotKnob squelchKnob(PIN_SQUELCHKNOB, 2, 1023, 30.0, 1.0); //Notice the reversed knob values, knobs installed upside down
    PotKnob ephemeralVolumeKnob(PIN_TIMEKNOB, 2, 1023, 30.0, 0.0); //Notice the reversed knob values, knobs installed upside down
    FlashStorage(mp3_volume_storage, int);

    void initHardware(EventHandler eventHandler) {

        pinMode(PIN_USER_OVERRIDE_BUTTON, INPUT_PULLUP);
        ButtonConfig* buttonConfig = overrideButton.getButtonConfig();
        buttonConfig->setClickDelay(300);
        buttonConfig->setDoubleClickDelay(600);
        buttonConfig->setLongPressDelay(1000);
        buttonConfig->setEventHandler(eventHandler);
        buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
        buttonConfig->setFeature(ButtonConfig::kFeatureClick);
        buttonConfig->setFeature(ButtonConfig::kFeatureSuppressClickBeforeDoubleClick);
        buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
    
        pinMode(PIN_LED_POWER_INDICATOR, OUTPUT);
        pinMode(PIN_LED_TRIGGER_INDICATOR, OUTPUT);
        digitalWrite(PIN_LED_POWER_INDICATOR, HIGH);
        digitalWrite(PIN_LED_TRIGGER_INDICATOR, HIGH);
    
        pinMode(PIN_TIMEKNOB, INPUT);
        pinMode(PIN_SQUELCHKNOB, INPUT);
        pinMode(PIN_EMF_ANTENNA, INPUT);
    
        pinMode(PIN_RELAY1, OUTPUT);
        pinMode(PIN_RELAY2, OUTPUT); 
        digitalWrite(PIN_RELAY1, LOW);
        digitalWrite(PIN_RELAY2, LOW);
    
        Serial.begin(115200);
        delay(2000);    
        Serial.println("Starting up...");
    
        Serial1.begin(9600); // Serial1 is used to communicate with DFPlayer
        delay(200);
        dfplay::initDfPlayer(getVolumeStorage());
    }

    void doLoop() {
        timer.run();
        overrideButton.check();
    }

    void relaysOn() {
        Serial.print("Turning on relays at: ");  Serial.println(millis());    
        digitalWrite(PIN_RELAY1, HIGH);
        digitalWrite(PIN_RELAY2, HIGH);
    }

    void relaysOff() {
        Serial.print("Turning off relays at: "); Serial.println(millis());
        digitalWrite(PIN_RELAY1, LOW);
        digitalWrite(PIN_RELAY2, LOW);
    }

    void setVolumeStorage(int volume) {
        //This is shifted by 1 because 0 means the storage has never been set
        Serial.print("setVolumeStorage: ");
        Serial.println(volume);
        mp3_volume_storage.write(volume + 1);
        Serial.println("setVolumeStorage done");
    }

    int getVolumeStorage() {
        int volume = mp3_volume_storage.read();
        Serial.print("getVolumeStorage (offset +1): ");
        Serial.println(volume);
        if (volume < 1) { //Default condition
            volume = MP3_VOLUME_DEFAULT;
            Serial.println("getVolumeStorage - default condition");
            setVolumeStorage(volume);
        } else { //Not default condition
            volume = volume - 1; //Shift back to 0-30 range
        }

        if (volume < 0) {
            volume = 0;
        } else if (volume > 30) {
            volume = 30;
        }

        return volume;
    }

    int getRelayOnDurationMs() {
        return relayOnTimeKnob.getMappedValue();
    }

    void userPlayRandomTrack() {
        dfplay::setVolume(getVolumeStorage());
        dfplay::playRandomTrack();
        displayUserEvent();
    }

    void userSetVolume() {
        int volume = ephemeralVolumeKnob.getMappedValue();
        Serial.print("userSetVolume: ");
        Serial.println(volume);
        setVolumeStorage(volume);
        dfplay::setVolume(volume);
        displayUserEvent();
    }

    /**
     * Just lets the user know that their action was registered.
     */
    void displayUserEvent() {
        digitalWrite(PIN_LED_POWER_INDICATOR, HIGH);
        digitalWrite(PIN_LED_TRIGGER_INDICATOR, LOW);
        delay(200);
        digitalWrite(PIN_LED_POWER_INDICATOR, LOW);
        digitalWrite(PIN_LED_TRIGGER_INDICATOR, HIGH);
        delay(200);
        digitalWrite(PIN_LED_POWER_INDICATOR, LOW);
        digitalWrite(PIN_LED_TRIGGER_INDICATOR, LOW);
    }

    bool isEmfDetected() {
        return ncv::isEmfDetected(squelchKnob.getMappedValue());
    }
} // namespace action