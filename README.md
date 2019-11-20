Adalight WS2812
===============

This is a fork of Adalight working with WS2811/WS2812 LED using the FastLED library (v 3.1).

FastLED library can be found on Github here : https://github.com/FastLED/FastLED


## Modification
This fork has the feature to detect actual light intencity and adapt Prismatik overall brightness according to it, this is done by using voltage divider between a 10k resistor and a photoresistor.

The voltage is read by analog pin at A7

#### Hardware update

+5v ________
            |
            |
        (10k resistor)
            |
A7  ________
            |
        (photresistor)
            |
            |
           GND

#### Software update
* After doing the hardware update and flashed your device with this sketch, enable Expert mode in Prismatik Lightpack and enable server (make sure it uses port 3636)
* Compile and execute the program [PrismatikAdapBrightness][https://github.com/cobrce/PrismatikAdapBrightness]