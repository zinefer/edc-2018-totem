#include "FastLED.h"

FASTLED_USING_NAMESPACE

#define DATA0_PIN          10
#define DATA1_PIN          11
#define DATA2_PIN          12
#define CLK_PIN            13
#define LED_TYPE           APA102
#define COLOR_ORDER        GRB
#define NUM_STRIPS         3
#define NUM_LEDS_PER_STRIP 100
#define NUM_LEDS           NUM_STRIPS * NUM_LEDS_PER_STRIP

#define BRIGHTNESS         25
#define FRAMES_PER_SECOND  120

CRGB leds[NUM_STRIPS][NUM_LEDS_PER_STRIP];

void setup() {
  // 3 second delay for recovery
  delay(3000);

  FastLED.addLeds<LED_TYPE,DATA0_PIN,CLK_PIN,COLOR_ORDER>(leds[0], NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE,DATA1_PIN,CLK_PIN,COLOR_ORDER>(leds[1], NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE,DATA2_PIN,CLK_PIN,COLOR_ORDER>(leds[2], NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);

  FastLED.setBrightness(BRIGHTNESS);
}

typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm, fire, levels };

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
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns);
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
// meteor
// pulse
// lightening
// theaterchase, rainbow
// pacman
// snake
// snow
// swirl chase, theater, rainbow, cylon?
// stripes

void fire()
{
  Fire2012();
  mirrorAlongY();
}

void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds[0], NUM_LEDS_PER_STRIP, gHue, 7);
  mirrorAlongY();
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
  for(int x = 0; x < NUM_STRIPS; x++) {
    if( random8() < chanceOfGlitter) {
      leds[x][ random16(NUM_LEDS_PER_STRIP) ] += CRGB::White;
    }
  }
}

void levels()
{
  // Randomly generate a percentage, set leds from 0->percentage to bright
  // let the rest fade to black
  for(int x = 0; x < NUM_STRIPS; x++) {
    fadeToBlackBy(leds[x], NUM_LEDS_PER_STRIP, 10);
    int pos = random8(35, 250);
    for(int y = 0; y < NUM_LEDS_PER_STRIP * (pos/255); y++) {
      leds[x][y] = CHSV( gHue, 255, 192);
    }
  }
}

void theaterRainbowChase()
{

}

void confetti()
{
  // random colored speckles that blink in and fade smoothly
  for(int x = 0; x < NUM_STRIPS; x++) {
    fadeToBlackBy( leds[x], NUM_LEDS_PER_STRIP, 10);
    int pos = random16(NUM_LEDS_PER_STRIP);
    leds[x][pos] += CHSV( gHue + random8(64), 200, 255);
  }
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds[0], NUM_LEDS_PER_STRIP, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS_PER_STRIP-1 );
  leds[0][pos] += CHSV( gHue, 255, 192);
  mirrorAlongY();
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
    leds[0][i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
  mirrorAlongY();
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds[0], NUM_LEDS_PER_STRIP, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[0][beatsin16( i+7, 0, NUM_LEDS_PER_STRIP-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
  mirrorAlongY();
}
