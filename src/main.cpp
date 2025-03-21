#include <Arduino.h>
#include <SimpleTimer.h>
#include "ncv.h"
#include "dfplay.h"

SimpleTimer timer;

#define PIN_TIMEKNOB A9
#define PIN_SQUELCHKNOB A8
#define PIN_LED_POWER_INDICATOR A1
#define PIN_LED_TRIGGER_INDICATOR A2
#define PIN_RELAY1 A3
#define PIN_RELAY2 A4
#define MP3_RX D7
#define MP3_TX D6


float timeKnobValue = 1.0;
float squelchKnobValue = 1.0;
long monitoredInputOnSince = 0;

const int monitoredInputTriggerThresholdMS = 5000;
const int minimumRelayOnMS = 2000;
const int maximumRelayOnMS = 15000;

const int musicalBreakDurationMS = 10000;

bool isEmfScanningOn = false;

float mapFloatVal(float x, float in_min, float in_max, float out_min, float out_max) {
  return (float)(x - in_min) * (out_max - out_min) / (float)(in_max - in_min) + out_min;
}

float getTimeKnobValue() {
    int lowVal = 2;
    int highVal = 1023;
    timeKnobValue = mapFloatVal(constrain(analogRead(PIN_TIMEKNOB), lowVal, highVal), lowVal, highVal, 0.0, 1.0);  

    Serial.print("Time knob value: ");  Serial.println(timeKnobValue);
    return timeKnobValue;
}

float getSquelchKnobValue() {
    int lowVal = 2;
    int highVal = 1023;
    squelchKnobValue = mapFloatVal(constrain(analogRead(PIN_SQUELCHKNOB), lowVal, highVal), lowVal, highVal, 0.0, 1.0);  

    // Serial.print("getSquelchKnobValue: ");  Serial.println(squelchKnobValue);
    return squelchKnobValue;
}

int getRelayOnDurationMS() {
    int result = (int)mapFloatVal(getTimeKnobValue(), 0.0, 1.0, minimumRelayOnMS, maximumRelayOnMS);
    Serial.print("getRelayOnDurationMS: ");  Serial.println(result);
    return result;
}

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
    pinMode(PIN_TIMEKNOB, INPUT);
    pinMode(PIN_LED_POWER_INDICATOR, OUTPUT);
    pinMode(PIN_LED_TRIGGER_INDICATOR, OUTPUT);
    pinMode(PIN_EMF_ANTENNA, INPUT);

    pinMode(PIN_RELAY1, OUTPUT);
    pinMode(PIN_RELAY2, OUTPUT); 
    digitalWrite(PIN_RELAY1, LOW);
    digitalWrite(PIN_RELAY2, LOW);

    Serial.begin(115200);       
    Serial.println("Starting up...");

    Serial1.begin(9600); // Serial1 is used to communicate with DFPlayer
    dfplay::initDfPlayerWithRetry(25);

    timer.setInterval(1000, [](){
        if (!isEmfScanningOn) {
            return;
        }

        long now = millis();
        if (ncv::isEmfDetected(getSquelchKnobValue())) {
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
    
                    timer.setTimeout(getRelayOnDurationMS(), [](){
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