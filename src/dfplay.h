#ifndef DFPLAY_H
#define DFPLAY_H

#include <Arduino.h>
#include "DFRobotDFPlayerMini.h"

namespace dfplay {
    extern DFRobotDFPlayerMini myDFPlayer;

    bool initDfPlayer(int volume);
    bool isDfPlayerAvailable();
    void playTrack(int track);
    void playRandomTrack();
    void stop();
    void setVolume(int volume);

}

#endif // DFPLAY_H