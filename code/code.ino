#include "FastLED.h"

FASTLED_USING_NAMESPACE

#define DATA0_PIN          10
#define DATA1_PIN          11
#define DATA2_PIN          12
#define CLK_PIN            13
#define LED_TYPE           APA102
#define COLOR_ORDER        BGR
#define NUM_STRIPS         3
#define NUM_LEDS_PER_STRIP 100
#define NUM_LEDS           NUM_STRIPS * NUM_LEDS_PER_STRIP

#define BRIGHTNESS         25
#define FRAMES_PER_SECOND  120

CRGB leds[NUM_STRIPS][NUM_LEDS_PER_STRIP];

void setup() {
  // 3 second delay for recovery
  delay(3000);

  FastLED.addLeds<LED_TYPE,DATA0_PIN,CLK_PIN,COLOR_ORDER,DATA_RATE_MHZ(12)>(leds[0], NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE,DATA1_PIN,CLK_PIN,COLOR_ORDER,DATA_RATE_MHZ(12)>(leds[1], NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE,DATA2_PIN,CLK_PIN,COLOR_ORDER,DATA_RATE_MHZ(12)>(leds[2], NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);

  FastLED.setBrightness(BRIGHTNESS);
}

typedef void (*SimplePatternList[])(bool);
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm, fire, meteor, levels, swirl, beatPulse, lightning, theaterRainbowChase };

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

void mirrorAlongY()
{
  for(int y = 0; y < NUM_LEDS_PER_STRIP; y++) {
    leds[1][y] = leds[0][y];
    leds[2][y] = leds[0][y];
  }
}

void noSetup()
{

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
    fill_rainbow(leds[0], NUM_LEDS_PER_STRIP, gHue, rDelta);
    mirrorAlongY();
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
  mirrorAlongY();
}

// "Drip" a pixel down the strand, leaving behind a variably decaying trail
int position = 0;
uint8_t decay[NUM_LEDS_PER_STRIP];
void meteor(bool setup)
{
  if (setup) return;

  EVERY_N_MILLISECONDS( 20 ) {
    position = (position + 1) % NUM_LEDS_PER_STRIP;

    for(int y = 0; y < NUM_LEDS_PER_STRIP; y++) {
      leds[0][y].fadeToBlackBy(decay[y]);
    }

    leds[0][position] += CHSV(gHue, 255, 255);
    decay[y-1] = random8(5,15);
  }
}

// Randomly generate a percentage, set leds from 0->percentage to bright
// let the rest fade to black
void levels(bool setup)
{
  if (setup) return;

  uint8_t BeatsPerMinute = 62;
  uint8_t beat = beatsin8(BeatsPerMinute, 0, 1);

  if (beat == 0) return;

  for(int x = 0; x < NUM_STRIPS; x++) {
    fadeToBlackBy(leds[x], NUM_LEDS_PER_STRIP, 10);

    int pos = random8(35, 250);

    for(int y = 0; y < NUM_LEDS_PER_STRIP * pos / 255; y++) {
      leds[x][y] = CHSV(gHue, 255, 192);
    }
  }
}

// a colored dot sprialing up with fading trails
// kinda sucks ... need to try bigger particle size
void swirl(bool setup)
{
  if (setup) return;

  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  int count = 0;

  for(int x = 0; x < NUM_STRIPS; x++) {
    fadeToBlackBy(leds[x], NUM_LEDS_PER_STRIP, 20);
  }

  for(int y = 0; y < NUM_LEDS_PER_STRIP; y++) {
    for(int x = 0; x < NUM_STRIPS; x++) {
      if (count++ == pos) {
        leds[x][y] += CHSV(gHue, 255, 192);
      }
    }
  }
}

// Slowly pulse a single color - TODO: maybe some random flickers?
void beatPulse(bool setup)
{
  if (setup) return;

  uint8_t BeatsPerMinute = 32;

  for(int x = 0; x < NUM_STRIPS; x++) {
    for(int y = 0; y < NUM_LEDS_PER_STRIP; y++) {
      uint8_t beat = beatsin8(BeatsPerMinute, 1, 254);
      leds[x][y] = CHSV(constrain(gHue, 1, 255), 255, 255-beat);
    }
  }
}

// Add thunder
void lightning(bool setup)
{
  if (setup) return;

  for(int x = 0; x < NUM_STRIPS; x++) {
    fadeToBlackBy(leds[x], NUM_LEDS_PER_STRIP, 50);

    if (pattern_duration/69 % 17 == 0) {
      for(int y = 0; y < NUM_LEDS_PER_STRIP; y++) {
        leds[x][y] += CRGB::White;
      }
    }
  }
}

void theaterRainbowChase(bool setup)
{
  if (setup) return;

  int offset = (pattern_duration / 75) % 3;

  rainbow(false);

  for(int x = 0; x < NUM_STRIPS; x++) {
    for(int y = 0; y < NUM_LEDS_PER_STRIP; y++) {
      if (y % 3 == 0) {
        leds[x][y + offset] = CRGB::Black;
      }
    }
  }
}

// random colored speckles that blink in and fade smoothly
void confetti(bool setup)
{
  if (setup) return;

  for(int x = 0; x < NUM_STRIPS; x++) {
    fadeToBlackBy(leds[x], NUM_LEDS_PER_STRIP, 10);

    int pos = random16(NUM_LEDS_PER_STRIP);
    leds[x][pos] += CHSV(gHue + random8(64), 200, 255);
  }
}

// a colored dot sweeping back and forth, with fading trails
void sinelon(bool setup)
{
  if (setup) return;

  fadeToBlackBy(leds[0], NUM_LEDS_PER_STRIP, 20);

  int pos = beatsin16(13, 0, NUM_LEDS_PER_STRIP-1 );
  leds[0][pos] += CHSV(gHue, 255, 192);

  mirrorAlongY();
}

// colored stripes pulsing at a defined Beats-Per-Minute (BPM)
void bpm(bool setup)
{
  if (setup) return;

  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;

  uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);

  for(int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
    leds[0][i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }

  mirrorAlongY();
}

// eight colored dots, weaving in and out of sync with each other
void juggle(bool setup) {
  if (setup) return;

  fadeToBlackBy(leds[0], NUM_LEDS_PER_STRIP, 20);

  byte dothue = 0;

  for(int i = 0; i < 8; i++) {
    leds[0][beatsin16( i+7, 0, NUM_LEDS_PER_STRIP-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }

  mirrorAlongY();
}

// == Sub effects

// Random sparkles
void addGlitter(fract8 chanceOfGlitter)
{
  for(int x = 0; x < NUM_STRIPS; x++) {
    if(random8() < chanceOfGlitter) {
      leds[x][ random16(NUM_LEDS_PER_STRIP) ] += CRGB::White;
    }
  }
}
