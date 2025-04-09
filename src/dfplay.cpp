#include "dfplay.h"

namespace dfplay {
    DFRobotDFPlayerMini myDFPlayer;

    void printDfPlayerDetail(uint8_t type, int value) {
        switch (type) {
            case TimeOut:
                Serial.println(F("Time Out!"));
                break;
            case WrongStack:
                Serial.println(F("Stack Wrong!"));
                break;
            case DFPlayerCardInserted:
                Serial.println(F("Card Inserted!"));
                break;
            case DFPlayerCardRemoved:
                Serial.println(F("Card Removed!"));
                break;
            case DFPlayerCardOnline:
                Serial.println(F("Card Online!"));
                break;
            case DFPlayerUSBInserted:
                Serial.println("USB Inserted!");
                break;
            case DFPlayerUSBRemoved:
                Serial.println("USB Removed!");
                break;
            case DFPlayerPlayFinished:
                Serial.print(F("Number:"));
                Serial.print(value);
                Serial.println(F(" Play Finished!"));
                break;
            case DFPlayerError:
                Serial.print(F("DFPlayerError:"));
                switch (value) {
                    case Busy:
                        Serial.println(F("Card not found"));
                        break;
                    case Sleeping:
                        Serial.println(F("Sleeping"));
                        break;
                    case SerialWrongStack:
                        Serial.println(F("Get Wrong Stack"));
                        break;
                    case CheckSumNotMatch:
                        Serial.println(F("Check Sum Not Match"));
                        break;
                    case FileIndexOut:
                        Serial.println(F("File Index Out of Bound"));
                        break;
                    case FileMismatch:
                        Serial.println(F("Cannot Find File"));
                        break;
                    case Advertise:
                        Serial.println(F("In Advertise"));
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }

    bool initDfPlayer(int volume) {
        Serial.println();
        Serial.println(F("DFRobot DFPlayer Mini Demo"));
        Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

        if (!myDFPlayer.begin(Serial1, /*isACK = */true, /*doReset = */true)) {  // Use serial to communicate with mp3.
            Serial.println(F("Unable to begin:"));
            Serial.println(F("1.Please recheck the connection!"));
            Serial.println(F("2.Please insert the SD card!"));
            return false;
        } else {
            Serial.println(F("DFPlayer Mini online."));

            if (myDFPlayer.available()) {
                printDfPlayerDetail(myDFPlayer.readType(), myDFPlayer.read());
            } else {
                Serial.println(F("DFPlayer Mini possibly not initialized."));
            }

            myDFPlayer.volume(volume);  // Set volume value. From 0 to 30
            return true;
        }
    }

    bool isDfPlayerAvailable() {
        return myDFPlayer.available();
    }

    void playTrack(int track) {
        if (!isDfPlayerAvailable()) {
            //HERE HERE!
            initDfPlayer(10);
        }
        Serial.print("Playing track: ");
        myDFPlayer.play(track);
        Serial.println(track);
    }

    void playRandomTrack(int trackMin, int trackMaxInclusive) {
        int track = random(trackMin, trackMaxInclusive + 1);
        playTrack(track);
    }

    void stop() {
        myDFPlayer.stop();
    }
}