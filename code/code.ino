#include "FastLED.h"

FASTLED_USING_NAMESPACE

#define DATA_PIN             11
#define CLK_PIN              13
#define LED_TYPE             APA102
#define COLOR_ORDER          BGR
#define FRAMES_PER_SECOND    120 

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
  // Baton specific config
  #define NUM_LEDS           10
  #define BRIGHTNESS         50
#endif

#if !(defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__))
  // Bigboi config
  #define NUM_LEDS           145
  #define BRIGHTNESS         25
#endif

CRGB leds[NUM_LEDS];

void setup() {
  // 3 second delay for recovery
  delay(3000);

  FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER,DATA_RATE_MHZ(12)>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  FastLED.setBrightness(BRIGHTNESS);
}

typedef void (*SimplePatternList[])(bool);
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm, fire, levels, beatPulse, lightning, theaterRainbowChase };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0;                  // rotating "base color" used by many of the patterns

unsigned long frame_start;
unsigned long pattern_start = 0;
unsigned long pattern_duration = 0;

void loop()
{
  frame_start = millis();

  // Update pattern duration before going into the pattern
  pattern_duration = millis() - pattern_start;

  gPatterns[gCurrentPatternNumber](false); // Update the leds array (prepare the frame)

  #if !(defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__))
    // The bigboi has a few too many leds, here we're going to turn off a few of the early ones
    for (int i = 0; i <= 16; i = i + 2) {
      leds[i] = CRGB::Black;
    }
  #endif
  
  FastLED.show();                          // Show the frame

  // Manage framerate
  unsigned long frame_duration = millis() - frame_start;
  FastLED.delay((1000 - frame_duration)/FRAMES_PER_SECOND);

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; }   // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 20 ) { nextPattern(); } // change patterns periodically
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  uint8_t nextPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns);
  pattern_start = millis();
  gPatterns[gCurrentPatternNumber](true); // Run pattern setup
  gCurrentPatternNumber = nextPatternNumber;
}

// fireworks
// snow
// swirl chase, theater, rainbow, cylon?
// stripes

// == Main effects

// FastLED's built-in rainbow generator
byte rDelta;
void rainbow(bool setup)
{
  if (setup) {
    rDelta = random8(5,35); // Width of the rainbow bands
  } else {
    fill_rainbow(leds, NUM_LEDS, gHue, rDelta);
  }
}

// rainbow from above, plus some random sparkly glitter
void rainbowWithGlitter(bool setup)
{
  if (setup) return;

  rainbow(false);
  addGlitter(80);
}

// Fire2012 by Mark Kriegsman
void fire(bool setup)
{
  if (setup) return;

  Fire2012();
}

// "Drip" a pixel down the strand, leaving behind a variably decaying trail
int position = 0;
uint8_t decay[NUM_LEDS];
void meteor(bool setup)
{
  if (setup) return;

  EVERY_N_MILLISECONDS( 20 ) {
    position = (position + 1) % NUM_LEDS;

    for(int y = 0; y < NUM_LEDS; y++) {
      leds[y].fadeToBlackBy(decay[y]);
    }

    int pos = NUM_LEDS - position; // :(

    leds[pos] += CHSV(gHue, 255, 255);
    leds[pos+1] += CHSV(gHue, 255, 255);
    leds[pos-1] %= random8(100,150);
    decay[pos-1] = random8(15,35);
  }
}

// Randomly generate a percentage, set leds from 0->percentage to bright
// let the rest fade to black
int levels_pos;
void levels(bool setup)
{
  if (setup) return;

  EVERY_N_MILLISECONDS( 625 ) {
    levels_pos = random16(NUM_LEDS / 2, NUM_LEDS - 5);
  }

    fadeToBlackBy(leds, NUM_LEDS, 25);

  for(int y = 0; y < levels_pos; y++) {
    leds[y] = CHSV(gHue, 255, 192);
  }
}

// Slowly pulse a single color - TODO: maybe some random flickers?
void beatPulse(bool setup)
{
  if (setup) return;

  uint8_t BeatsPerMinute = 32;

  for(int y = 0; y < NUM_LEDS; y++) {
    uint8_t beat = beatsin8(BeatsPerMinute, 1, 254);
    leds[y] = CHSV(constrain(gHue, 1, 255), 255, 255-beat);
  }
}

// Add thunder
int pulse_position;
void lightning(bool setup)
{
  if (setup) {
    pulse_position = 0;
    return;
  }

  EVERY_N_MILLISECONDS( 20 ) {

    int segment = NUM_LEDS / 5;

    fadeToBlackBy(leds, NUM_LEDS, 50);

    if (pulse_position > 0) {

      switch(pulse_position++) {
        case 2:
          for(int y = 4 * segment; y > segment; y--) {
            leds[y] += CRGB::White;
          }
        break;
        case 3:
          for(int y = segment; y > 0; y--) {
            leds[y] += CRGB::White;
          }
          pulse_position = 0;
        break;
      }

    } else {

      if (random8(0, 10) == 2) {
        for(int y = 5 * segment; y > 4 * segment; y--) {
          leds[y] += CRGB::White;
        }
        pulse_position = 1;
      }

    }
  }
}

void theaterRainbowChase(bool setup)
{
  if (setup) return;

  int offset = (pattern_duration / 75) % 3;

  rainbow(false);

  for(int y = 0; y < NUM_LEDS; y++) {
    if (y % 3 == 0) {
      leds[y + offset] = CRGB::Black;
    }
  }
}

// random colored speckles that blink in and fade smoothly
void confetti(bool setup)
{
  if (setup) return;

  fadeToBlackBy(leds, NUM_LEDS, 10);

  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV(gHue + random8(64), 200, 255);
}

// a colored dot sweeping back and forth, with fading trails
void sinelon(bool setup)
{
  if (setup) return;

  fadeToBlackBy(leds, NUM_LEDS, 20);

  int pos = beatsin16(13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV(gHue, 255, 192);
}

// colored stripes pulsing at a defined Beats-Per-Minute (BPM)
void bpm(bool setup)
{
  if (setup) return;

  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;

  uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);

  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

// eight colored dots, weaving in and out of sync with each other
void juggle(bool setup) {
  if (setup) return;

  fadeToBlackBy(leds, NUM_LEDS, 20);

  byte dothue = 0;

  for(int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

// == Sub effects

// Random sparkles
void addGlitter(fract8 chanceOfGlitter)
{
  if(random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}
