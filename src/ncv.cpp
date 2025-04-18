#include "ncv.h"
#include <iostream>

/**
 * Inspired by: https://www.instructables.com/Contactless-AC-Detector/
 */

namespace ncv {

    float emfVal;                            //where to store info from analog 6
    long averageADC = 0;
    float amplitudeScaleMax = 200.0;
    
    bool isEmfDetected(float squelchValue) {
        int16_t adc0;
        long sum, t;
        double c[200], s[200], amplitude, cp, sp, phi;
        int i;
        sum = 0;
        i = 0;
        bool isDetected = false;
        while (i < EMF_SAMPLE_COUNT) {
            adc0 = analogRead(PIN_EMF_ANTENNA) - 1023 * 3 / 5;
            t = micros();
            phi = 6.2831853 * t / 1.0e6 * POWER_HZ;
            c[i] = (adc0 - averageADC)  * cos(phi);
            s[i] = (adc0 - averageADC)  * sin(phi);
            sum += adc0;
            i++;
        }
        averageADC = (sum / EMF_SAMPLE_COUNT) ;
        i = 0; cp = 0; sp = 0;
        while (i < EMF_SAMPLE_COUNT) {
            cp += c[i] / EMF_SAMPLE_COUNT;
            sp += s[i] / EMF_SAMPLE_COUNT;
            i++;
        }
        amplitude =  sqrt(cp * cp + sp * sp);
        emfVal = amplitude;                 
        emfVal = constrain(emfVal, 0, amplitudeScaleMax);       
    
        isDetected = emfVal > squelchValue;
    
        Serial.print("isDetected: ");
        Serial.print(isDetected ? "+" : "-");
        Serial.print(" | ");
        Serial.print("EMF Amplitude: ");
        Serial.print(amplitude);
        Serial.print(" | ");
        Serial.print(" adc: ");
        Serial.print(adc0);
        Serial.print(" | ");
        Serial.print(" squ: ");
        Serial.print(squelchValue);
        Serial.println();
    
        return isDetected;
    }

} // namespace ncv