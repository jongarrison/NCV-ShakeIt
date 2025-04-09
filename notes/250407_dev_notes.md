

4/7/25 Received the boards and assembled them. They work, BUT...

- Both speaker pins on the DF Mini Player should have been exposed via the header outputs
- An override switch should have been included
- Some sort of access to the SAMD21 reset is needed. Perhaps some terminals to pads for a switch? Perhaps have jumper wires from the topside reset contacts to the terminals
- Expose all of the pins of the SAMD21 with through hole connectors alongside the existing pin pads? This would allow easier repurposing of the boards.
- Replace the 1M resistor with a 1M trim pot?






Notes from initializing the 

        bool initDfPlayerWithRetry(int volume) {
            //Serial1.begin(9600); // Must be done in setup()

            while (!initDfPlayer(volume)) {
                Serial.println(F("Retrying to initialize DFPlayer ..."));
                delay(5000);
            }

            if (myDFPlayer.available()) {
                printDfPlayerDetail(myDFPlayer.readType(), myDFPlayer.read());
            }

            return true;
