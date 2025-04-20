#include <Arduino.h>
#include <SimpleTimer.h>
#include "ncv.h"
#include "dfplay.h"
#include "potknob.h"
#include <FlashStorage.h>
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
#define RELAY_TRIGGER_DELAY_MS 10000
#define MINIMUM_NCV_ON_TIME_MS 5000
#define PIN_OVERRIDE_BUTTON 0
#define POT_KNOB_MIN_MS 2000
#define POT_KNOB_MAX_MS 30000

long monitoredInputOnSince = 0;
bool isEmfScanningOn = false;
int activeMp3Volume = MP3_VOLUME_DEFAULT;

SimpleTimer timer;
AceButton overrideButton(static_cast<double>(PIN_OVERRIDE_BUTTON));
PotKnob relayOnTimeKnob(PIN_TIMEKNOB, 2, 1023, POT_KNOB_MAX_MS, POT_KNOB_MIN_MS); //Notice the reversed knob values, knob installed upside down
PotKnob squelchKnob(PIN_SQUELCHKNOB, 2, 1023, 40.0, 2.0);
PotKnob ephemeralVolumeKnob(PIN_TIMEKNOB, 2, 1023, 30.0, 0.0);
FlashStorage(mp3_volume_storage, int);

void activateRelaySequence() {
    Serial.print("Turning on relays at: ");  Serial.println(millis());
    isEmfScanningOn = false;

    digitalWrite(PIN_RELAY1, HIGH);
    digitalWrite(PIN_RELAY2, HIGH);

    timer.setTimeout((long)relayOnTimeKnob.getMappedValue(), [](){
        Serial.print("Turning off relays at: "); Serial.println(millis());
        digitalWrite(PIN_RELAY1, LOW);
        digitalWrite(PIN_RELAY2, LOW);

        Serial.println("Starting the EMF scanning again...");
        isEmfScanningOn = true;
    });
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

void handleOverrideButton(AceButton* /*button*/, uint8_t eventType, uint8_t /*buttonState*/) {
    Serial.print("Override button event: ");
    Serial.println(eventType);

    if (eventType == AceButton::kEventClicked) {
        Serial.println("kEventClick - Activating relay sequence");
        activateRelaySequence();
        displayUserEvent();
    } else if (eventType == AceButton::kEventDoubleClicked) {
        Serial.println("kEventDoubleClicked - Playing random track");
        dfplay::setVolume(getVolumeStorage());
        dfplay::playRandomTrack();
        displayUserEvent();
    } else if (eventType == AceButton::kEventLongPressed) {
        Serial.println("Setting volume based on reuse of Timing Knob");
        int volSet = (int)ephemeralVolumeKnob.getMappedValue();
        setVolumeStorage(volSet);
        dfplay::setVolume(volSet);
        displayUserEvent();
    }
}

void setup() {
    pinMode(PIN_OVERRIDE_BUTTON, INPUT_PULLUP);
    ButtonConfig* buttonConfig = overrideButton.getButtonConfig();
    buttonConfig->setClickDelay(250);
    buttonConfig->setDoubleClickDelay(500);
    buttonConfig->setLongPressDelay(1000);
    buttonConfig->setEventHandler(handleOverrideButton);
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

    timer.setInterval(1000, [](){
        if (!isEmfScanningOn) {
            return;
        }

        // Serial.println("Relay on time work:");
        // float relayOnTime = relayOnTimeKnob.getMappedValue();
        // Serial.println("Squelch work:");
        float squelchValue = squelchKnob.getMappedValue();

        long now = millis();
        if (ncv::isEmfDetected(squelchValue)) {
            if (monitoredInputOnSince == 0) {
                monitoredInputOnSince = now;
                Serial.print("Monitored input is first ON at: ");  Serial.println(now);
            } else {
                Serial.print("Monitored input is still ON at: ");  Serial.println(now);
            }
            digitalWrite(PIN_LED_POWER_INDICATOR, LOW);
            digitalWrite(PIN_LED_TRIGGER_INDICATOR, HIGH);            
        } else { //Monitored input is OFF
            digitalWrite(PIN_LED_TRIGGER_INDICATOR, LOW);            
            digitalWrite(PIN_LED_POWER_INDICATOR, HIGH);

            if (monitoredInputOnSince != 0 && now - monitoredInputOnSince > MINIMUM_NCV_ON_TIME_MS) {                
                Serial.println("Starting the music with included machine start delay");
                isEmfScanningOn = false;

                timer.setTimeout(50, [](){
                    Serial.print("Starting the music at: ");  Serial.println(millis());
                    dfplay::setVolume(getVolumeStorage());
                    dfplay::playRandomTrack();

                    timer.setTimeout(RELAY_TRIGGER_DELAY_MS, [](){
                        Serial.print("Stopping the music at: ");  Serial.println(millis());
                        dfplay::stop();
                    });
                });

                timer.setTimeout(RELAY_TRIGGER_DELAY_MS + 500, [](){
                    activateRelaySequence();
                });
            }            
            monitoredInputOnSince = 0;
        }

    });

    isEmfScanningOn = true;
    digitalWrite(PIN_LED_POWER_INDICATOR, HIGH);
}

void loop() {
    timer.run();
    overrideButton.check();
}

