#include "dfplay.h"

#define DFPLAY_MIN_TRACK 1
#define DFPLAY_MAX_TRACK 3


namespace dfplay {
    DFRobotDFPlayerMini myDFPlayer;

    bool isInitSuccessful = false;

    bool initDfPlayer(int volume) {
        delay(1000);
        Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

        if (!myDFPlayer.begin(Serial1, /*isACK = */true, /*doReset = */true)) {  // Use serial to communicate with mp3.
            Serial.println(F("Unable to start DF Player:"));
            Serial.println(F("1. Recheck the connection"));
            Serial.println(F("2. Check SD card is inserted"));

            isInitSuccessful = false;
            return false;
        } else {
            Serial.println(F("DFPlayer Mini serial connection successful"));
            isInitSuccessful = true;
            setVolume(volume);
            return true;
        }
    }

    bool isDfPlayerAvailable() {
        return isInitSuccessful;
    }

    /**
     * Expect there to be a folder named 01 on the SD card, and inside it
     * there should be files named 001.mp3, 002.mp3, etc.
     * more text can be after the 3 digit prefixes
     */
    void playTrack(int track) {
        if (!isInitSuccessful) {
            Serial.println(F("DFPlayer was not successfully initialized, skipping music"));
            return;
        }
        Serial.print("playTrack track: ");
        Serial.println(track);
        Serial.print("playTrack readVolume: ");
        Serial.println(myDFPlayer.readVolume());

        myDFPlayer.playFolder((int)1, track);
        Serial.println(("Play call done"));
    }

    void playRandomTrack() {
        randomSeed(millis());
        int track = random(DFPLAY_MIN_TRACK, DFPLAY_MAX_TRACK + 1);
        Serial.print("Chose random track: ");
        Serial.println(track);
        playTrack(track);
    }

    void stop() {
        if (!isInitSuccessful) {
            Serial.println(F("DFPlayer was not successfully initialized, skipping stop"));
            return;
        }
        myDFPlayer.stop();
    }

    void setVolume(int volume) {
        if (!isInitSuccessful) {
            Serial.println(F("DFPlayer was not successfully initialized, skipping volume change"));
            return;
        }
        Serial.print(F("setVolume setting: "));
        Serial.println(volume);
        myDFPlayer.volume(volume);

        // It's not clear that the readVolume actually works
        // delay(250);
        // Serial.print(F("setVolume read: "));
        // Serial.println(myDFPlayer.readVolume());
    }
}