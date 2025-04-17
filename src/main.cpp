#include <Arduino.h>
#include <SimpleTimer.h>
#include "ncv.h"
#include "dfplay.h"
#include "PotKnob.h"

#define PIN_TIMEKNOB A9
#define PIN_SQUELCHKNOB A8
#define PIN_LED_POWER_INDICATOR A1
#define PIN_LED_TRIGGER_INDICATOR A2
#define PIN_RELAY1 A3
#define PIN_RELAY2 A4
#define MP3_RX D7
#define MP3_TX D6
#define MP3_VOLUME 30
#include <AceButton.h>
using namespace ace_button;

const int PIN_OVERRIDE_BUTTON=D0;

long monitoredInputOnSince = 0;

const int monitoredInputTriggerThresholdMS = 5000;
const int minimumRelayOnMS = 2000;
const int maximumRelayOnMS = 30000;

const int musicalBreakDurationMS = 10000;

bool isEmfScanningOn = false;

SimpleTimer timer;
AceButton overrideButton(PIN_OVERRIDE_BUTTON);
PotKnob relayOnTimeKnob(PIN_TIMEKNOB, 2, 1023, maximumRelayOnMS, minimumRelayOnMS); //Notice the reversed knob values
PotKnob squelchKnob(PIN_SQUELCHKNOB, 2, 1023, 40.0, 2.0);

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

void handleOverrideButton(AceButton* /*button*/, uint8_t eventType, uint8_t /*buttonState*/) {
    if (eventType == AceButton::kEventPressed) {
        activateRelaySequence();
    }
}

void setup() {
    pinMode(PIN_OVERRIDE_BUTTON, INPUT_PULLUP);
    overrideButton.setEventHandler(handleOverrideButton);

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
    Serial.println("Starting up...");

    Serial1.begin(9600); // Serial1 is used to communicate with DFPlayer
    dfplay::initDfPlayer(MP3_VOLUME);

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

            if (monitoredInputOnSince != 0 && now - monitoredInputOnSince > monitoredInputTriggerThresholdMS) {                
                Serial.println("Starting the music with included machine start delay");
                isEmfScanningOn = false;

                timer.setTimeout(50, [](){
                    Serial.print("Starting the music at: ");  Serial.println(millis());
                    dfplay::playRandomTrack(1, 3, MP3_VOLUME);

                    timer.setTimeout(musicalBreakDurationMS, [](){
                        Serial.print("Stopping the music at: ");  Serial.println(millis());
                        dfplay::stop();
                    });
                });

                timer.setTimeout(musicalBreakDurationMS + 500, [](){
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

