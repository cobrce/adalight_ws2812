/*
 * Arduino interface for the use of WS2812 strip LEDs
 * Uses Adalight protocol and is compatible with Boblight, Prismatik etc...
 * "Magic Word" for synchronisation is 'Ada' followed by LED High, Low and Checksum
 * @author: Wifsimster <wifsimster@gmail.com> 
 * @library: FastLED v3.001
 * @date: 11/22/2015
 */

// #define ESP8266

#ifdef ESP8266
#define FASTLED_ESP8266_D1_PIN_ORDER
#endif

#include <FastLED.h>


#ifdef ESP8266
#define FASTLED_ESP8266_D1_PIN_ORDER
#define DATA_PIN D6
#define ANALOG_PIN A0
#define serialRate 1000000
#else
#define DATA_PIN 6
#define ANALOG_PIN A7
// Baudrate, higher rate allows faster refresh rate and more LEDs (defined in /etc/boblight.conf)
#define serialRate 250000
#endif
#define NUM_LEDS 80

// Adalight sends a "Magic Word" (defined in /etc/boblight.conf) before sending the pixel data
uint8_t prefix[] = {'A', 'd', 'a'}, hi, lo, chk, i;

// Initialise LED-array
CRGB leds[NUM_LEDS];

#define UPDATE_BRIGHTNESS_MANUALLY

void setup() {
  // Use NEOPIXEL to keep true colors
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  
  // Initial RGB flash
  LEDS.showColor(CRGB(255, 0, 0));
  delay(500);
  LEDS.showColor(CRGB(0, 255, 0));
  delay(500);
  LEDS.showColor(CRGB(0, 0, 255));
  delay(500);
  LEDS.showColor(CRGB(0, 0, 0));
  
  Serial.begin(serialRate);
  // Send "Magic Word" string to host
  Serial.print("Ada\n");

  pinMode(ANALOG_PIN,INPUT);
}

unsigned long prevUpdate = 0;
#define UPDATE_DELAY 100

void UpdateBrightness(bool force = false)
{
    #define N_SAMPLES 10
    static bool firstSample = true;
    static unsigned char samples[N_SAMPLES];
    static char sampleIndex = 0;
    static unsigned char prevMean = 0;

    unsigned long now = millis();
    if (force || prevUpdate == 0 || (now - prevUpdate)>=UPDATE_DELAY)
    {
        prevUpdate = now;
        int value = analogRead(ANALOG_PIN);

        #ifdef UPDATE_BRIGHTNESS_MANUALLY
        // convert from 900-100 scale to 20%-100% scale to 50-255 scale
        value = constrain(value,100,900);
        float ratio = (value - 100) * ((20.0 - 100.0) / (900.0-100.0)) + 100.0;
        ratio = constrain(ratio,20.0,100.0);
        ratio /=100.0;
        unsigned int scale = 255*ratio;
        scale = constrain(scale,50,255);

        // fill sample vector if it's the first sample        
        if (firstSample)
        {
            firstSample = false;
            for (int i = 0;i<N_SAMPLES;i++)
                samples[i] = scale;
        }

        // set current scale in sample vector
        // sampleIndex is a rotating index for samles vector
        // (a fast/lazy way to implement a queue)
        samples[sampleIndex] = scale;
        sampleIndex = (sampleIndex+1)%N_SAMPLES;

        // calculate mean of samples
        unsigned int sum = 0;
        for (int i = 0 ;i<N_SAMPLES;i++)
            sum+=samples[i];
        
        char mean = sum/N_SAMPLES;

        Serial.print((unsigned int)mean);
        Serial.println();
        // if mean changes by 5 update brigtness
        if (abs(prevMean-mean)>5)
        {
            prevMean = mean;
            FastLED.setBrightness(mean);
        }

        // display
        FastLED.show();
        #else
        Serial.println(value);
        #endif
    }
}

void loop() { 
  // Wait for first byte of Magic Word
  for(i = 0; i < sizeof prefix; ++i) {
    waitLoop: do{UpdateBrightness();} while (!Serial.available());
    // Check next byte in Magic Word
    if(prefix[i] == Serial.read()) continue;
    // otherwise, start over
    i = 0;
    goto waitLoop;
  }
  
  // Hi, Lo, Checksum  
  while (!Serial.available()) ;;
  hi=Serial.read();
  while (!Serial.available()) ;;
  lo=Serial.read();
  while (!Serial.available()) ;;
  chk=Serial.read();
  
  // If checksum does not match go back to wait
  if (chk != (hi ^ lo ^ 0x55)) {
    i=0;
    goto waitLoop;
  }
  memset(leds, 0, NUM_LEDS * sizeof(struct CRGB));
  // Read the transmission data and set LED values
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    byte r, g, b;    
    while(!Serial.available());
    r = Serial.read();
    while(!Serial.available());
    g = Serial.read();
    while(!Serial.available());
    b = Serial.read();
    leds[i].r = r;
    leds[i].g = g;
    leds[i].b = b;
  }
  #ifdef UPDATE_BRIGHTNESS_MANUALLY
  UpdateBrightness(true);
  #else
  // Shows new values
  FastLED.show();
  #endif
}
