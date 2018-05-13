#include "FastLED.h"


FASTLED_USING_NAMESPACE

#define DATA_PIN           11
#define CLK_PIN            13
#define LED_TYPE           APA102
#define COLOR_ORDER        GRB
#define NUM_LEDS           64
CRGB leds[NUM_LEDS];

#define BRIGHTNESS         25
#define FRAMES_PER_SECOND  120

void setup() {
  // 3 second delay for recovery
  delay(3000);

  FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  FastLED.setBrightness(BRIGHTNESS);
}

typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle,  bpm,     fire };

//typedef void (*SimpleSetupList[])();
//SimpleSetupList gSetups     = { noSetup, noSetup,            noSetup,  noSetup, noSetup, noSetup, noSetup };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0;                  // rotating "base color" used by many of the patterns

void loop()
{
  gPatterns[gCurrentPatternNumber]();      // Update the leds array (prepare the frame)
  FastLED.show();                          // Show the frame

  FastLED.delay(1000/FRAMES_PER_SECOND);   // Manage framerate

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; }   // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 20 ) { nextPattern(); } // change patterns periodically
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void noSetup()
{

}

// fireworks
// meteor
// pulse
// lightening
// theaterchase, rainbow
// levels
// pacman
// snake
// snow
// swirl chase, theater, rainbow, cylon?
// stripes

void fire()
{
  Fire2012();
}

void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter)
{
  // Random sparkles
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void theaterRainbowChase()
{

}

void confetti()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}
