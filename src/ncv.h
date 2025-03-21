#ifndef NCV_H
#define NCV_H

#include <Arduino.h>

namespace ncv {
    #define PIN_EMF_ANTENNA A5
    #define EMF_SAMPLE_COUNT 200                    //this is how many samples the device takes per reading
    #define POWER_HZ 60.0 //Replace 50.0 by 60.0 if your mains is 60 Hz

    bool isEmfDetected(float squelchValue);

} // namespace ncv

#endif // NCV_H