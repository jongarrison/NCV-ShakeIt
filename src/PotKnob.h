#ifndef POTKNOB_H
#define POTKNOB_H

#include <Arduino.h>

class PotKnob {
private:
    int adcPin;          // ADC pin numberint
    int inAdcLowVal;       // Minimum ADC input value
    int inAdcHighVal;      // Maximum ADC input value
    float minOutput;  // Minimum float output value
    float maxOutput;  // Maximum float output value

public:
    // Constructor to initialize the potentiometer parameters
    PotKnob(int adcPin, int adcLowVal = 2, int adcHighVal = 1023, float minOutput = 0.0, float maxOutput = 1.0)
        : adcPin(adcPin), inAdcLowVal(adcLowVal), inAdcHighVal(adcHighVal), minOutput(minOutput), maxOutput(maxOutput) {}

    // Method to map an ADC input value to the desired float range
    float getMappedValue() const {
        int adcValue = analogRead(adcPin); // Read the ADC value
        int constrainedValue = constrain(adcValue, inAdcLowVal, inAdcHighVal);

        float result = ((float)(constrainedValue - inAdcLowVal) * (maxOutput - minOutput) / (float)(inAdcHighVal - inAdcLowVal)) + minOutput;

        // Serial.print(adcPin);
        // Serial.print("- ");
        // Serial.print("Knob val: ");
        // Serial.print(result);
        // Serial.print(" | ");
        // Serial.print("ADC Value: ");
        // Serial.print(adcValue);
        // Serial.print(" | ");
        // Serial.print("Constrained Value: ");
        // Serial.print(constrainedValue);
        // Serial.print(" | ");
        // Serial.print("minOutput: ");
        // Serial.print(minOutput);
        // Serial.print(" | ");
        // Serial.print("maxOutput: ");
        // Serial.println(maxOutput);
        return result;
    }

    // Method to map an ADC input value to a percentage (0.0 to 1.0)
    float getPercentValue() const {
        int adcValue = analogRead(adcPin); // Read the ADC value
        int constrainedValue = constrain(adcValue, inAdcLowVal, inAdcHighVal);
        return (float)(constrainedValue - inAdcLowVal) / (float)(inAdcHighVal - inAdcLowVal);
    }
};

#endif // POTKNOB_H