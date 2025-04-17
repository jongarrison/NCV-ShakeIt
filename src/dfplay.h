#ifndef DFPLAY_H
#define DFPLAY_H

#include <Arduino.h>
#include "DFRobotDFPlayerMini.h"

namespace dfplay {
    extern DFRobotDFPlayerMini myDFPlayer;

    bool initDfPlayer(int volume);
    bool isDfPlayerAvailable();
    void playTrack(int track, int volume);
    void playRandomTrack(int trackMin, int trackMaxInclusive, int volume);
    void stop();
}

#endif // DFPLAY_H