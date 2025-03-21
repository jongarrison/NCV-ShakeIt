#ifndef DFPLAY_H
#define DFPLAY_H

#include <Arduino.h>
#include "DFRobotDFPlayerMini.h"

namespace dfplay {
    extern DFRobotDFPlayerMini myDFPlayer;

    bool initDfPlayer(int volume);
    bool initDfPlayerWithRetry(int volume);
    void playTrack(int track);
    void playRandomTrack(int trackMin, int trackMaxInclusive);
    void stop();
}

#endif // DFPLAY_H