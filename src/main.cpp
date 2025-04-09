#include <Arduino.h>
#include <SimpleTimer.h>
#include "ncv.h"
#include "dfplay.h"
#include "PotKnob.h"

SimpleTimer timer;

#define PIN_TIMEKNOB A9
#define PIN_SQUELCHKNOB A8
#define PIN_LED_POWER_INDICATOR A1
#define PIN_LED_TRIGGER_INDICATOR A2
#define PIN_RELAY1 A3
#define PIN_RELAY2 A4
#define MP3_RX D7
#define MP3_TX D6
#define MP3_VOLUME 30


long monitoredInputOnSince = 0;

const int monitoredInputTriggerThresholdMS = 5000;
const int minimumRelayOnMS = 2000;
const int maximumRelayOnMS = 30000;

const int musicalBreakDurationMS = 10000;

bool isEmfScanningOn = false;

PotKnob relayOnTimeKnob(PIN_TIMEKNOB, 2, 1023, maximumRelayOnMS, minimumRelayOnMS); //Notice the reversed knob values
PotKnob squelchKnob(PIN_SQUELCHKNOB, 2, 1023, 40.0, 2.0);

int getRandomTrack() {
    return random(1, 4); // Generate a random number between 1 and 3
    // int randomValue = random(0, 100); // Generate a random number between 0 and 99
    // if (randomValue < 95) {
    //     return 1;
    // } else if (randomValue < 97) {
    //     return 2;
    // } else {
    //     return 3;
    // }
}

void setup() {
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

        Serial.println("Relay on time work:");
        float relayOnTime = relayOnTimeKnob.getMappedValue();
        Serial.println("Squelch work:");
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
                    dfplay::playTrack(getRandomTrack());

                    timer.setTimeout(musicalBreakDurationMS, [](){
                        Serial.print("Stopping the music at: ");  Serial.println(millis());
                        dfplay::stop();
                    });
                });

                timer.setTimeout(musicalBreakDurationMS + 500, [](){

                    Serial.print("Turning on relays at: ");  Serial.println(millis());
                    digitalWrite(PIN_RELAY1, HIGH);
                    digitalWrite(PIN_RELAY2, HIGH);
    
                    timer.setTimeout((long)relayOnTimeKnob.getMappedValue(), [](){
                        Serial.print("Turning off relays at: "); Serial.println(millis());
                        digitalWrite(PIN_RELAY1, LOW);
                        digitalWrite(PIN_RELAY2, LOW);

                        Serial.println("Starting the EMF scanning again...");
                        isEmfScanningOn = true;
                    });

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
}