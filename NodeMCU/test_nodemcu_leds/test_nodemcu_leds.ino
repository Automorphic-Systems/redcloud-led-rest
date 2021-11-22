#include <FastLED.h>
#include <stdio.h>

#define LED_PIN 7
#define NUM_LEDS 288
#define UCHAR_MAX 255
#define FRAME_INTERVAL 32
#define HSV_DEFAULT_VALUE 160

/*
/* LED Operation Tests for NodeHost_MCU */
/*

/* LED and heatmap values*/
CRGB leds[NUM_LEDS];
uint8_t leds_h[NUM_LEDS];
CRGB seedLeds[NUM_LEDS];
CRGBPalette16 currentPalette;
static int heatmap[NUM_LEDS];

/* Logging */
char logbuffer [80];
int logsize;
char *modes[4] = {"rainbow", "streak", "chase", "flicker"};
bool cycle;
char *currentMode;

/* Timing */
unsigned long currTime, prevTime, startTime, cycleTime;

/* Mode Parameters */
uint8_t hue;
uint16_t pos;
int16_t offset;
uint8_t seeds, pal_cols;
int8_t chase_rate;
int16_t iter, maxiter;
uint8_t fade_rate, boost_rate, boost_thr, twink_thr, twink_min, twink_max, seed_amt;

/* Setup  */
void setup() {

  /* Set pins, clear UART */  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(D4, OUTPUT);
  delay(2000);
  Serial.begin(115200);
  delay(500);
  Serial.flush();
  while ( Serial.available() ) Serial.read(); 

  Serial.println("Initializing parameters, activating LEDs");

  /* General */
  pos, hue, currTime, prevTime, startTime, offset, iter = 0;
  pal_cols = 2;
  maxiter = NUM_LEDS;

  /* Chase */
  chase_rate = 2;

  /* Flicker */
  fade_rate = 6;
  boost_rate = 5;
  seed_amt = 20;
  boost_thr = 100;
  twink_thr = 32;
  twink_min = 48;
  twink_max = 160;

  /* Set LED strip */
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);

  /* Test MOSFET switch */
  //testMosfetSwitch();

  startTime = millis();
  cycleTime = millis();  
  currentPalette = generateRandomPalette(pal_cols);
  generateRandomFrameFromPalette(seedLeds);
  currentMode = modes[3];
}

/* Loop */
void loop() {
  currTime = millis();
  
  //testMillis();
  if (currTime - prevTime > FRAME_INTERVAL) {
     //testLEDsRainbowDraw();  
     //testLEDsSingleChaseDraw();
     //testLEDsFullChaseDraw();
     testFlickeringDraw();
  
     FastLED.show();       
     prevTime = currTime;      
  } 

  if (cycle) {    
    logsize = sprintf(logbuffer, "mode: %s, rate: %i, leds: %i, time: %i ms", currentMode, FRAME_INTERVAL, NUM_LEDS, millis() - cycleTime);
    Serial.println(logbuffer);
    cycleTime = millis();
    cycle = 0;
  }
}

/* Test LED load being switched on/off via MOSFET */
void testMosfetSwitch() {
  Serial.println("Activating power load.");
  int powCnt = 0;

  Serial.println("Drawing LEDs.");
  FastLED.showColor(CRGB(255,0,0),HSV_DEFAULT_VALUE);

  for (powCnt = 0; powCnt < 5; powCnt++) {
    Serial.println("Turning on.");
    digitalWrite(D4, LOW);
    delay(2000);
    
    FastLED.showColor(CRGB(128,0,128),128);
    delay(2000);
    
    Serial.println("Turning off.");
    digitalWrite(D4, HIGH); /* LEDs should go dark */    
    delay(2000);
  }

  Serial.println("Turning on.");
  digitalWrite(D4, LOW);
}


/* Test millis call */
void testMillis() {  
  if (currTime  - prevTime > 1000) {
    Serial.println("Incrementing second");
    prevTime = currTime;
  }
}


/* Rainbow mode test */
void testLEDsRainbowDraw() {
  FastLED.showColor(CHSV(hue++, 255, HSV_DEFAULT_VALUE));

  /* Reset condition */
  if (hue >= UCHAR_MAX) {
    hue = 0;
    cycle = 1;
  }
}


/* Chase mode test */
void testLEDsSingleChaseDraw() {
    leds[pos] = CHSV(hue, 0, 0);
    if (pos < NUM_LEDS - 1) {
       pos++;
    } else {      
       /* Reset condition */
       pos = 0;
       cycle = 1;
    }
    leds[pos] = CHSV(mod8(hue++,UCHAR_MAX), 255, 255);
}

/* Full chase mode test, with palettes */
void testLEDsFullChaseDraw() {    
    for(int i=0;i<NUM_LEDS;i++) {
        leds[i] = seedLeds[(i + offset)% NUM_LEDS];
    }
    offset = (offset+chase_rate+NUM_LEDS)%NUM_LEDS;    
    iter++;
    
    /* Reset condition */
    if (iter == maxiter) {
        iter = 0;
        offset = 0;      
        cycle = 1;
        generateRandomFrameFromPalette(seedLeds);
    } 
}

/* Flickering mode test, with palettes */
void testFlickeringDraw() {
    random16_add_entropy( random(UCHAR_MAX));
    seeds = random16(seed_amt, NUM_LEDS - seed_amt);

    /* Fading */
    for ( int i = 0; i < NUM_LEDS; i++) {
        heatmap[i] = qsub8( heatmap[i], fade_rate);
    }

    /* Twinkle */
    for ( int j = 0 ; j < seeds ; j++) {
     if (random8() < twink_thr) {
      //again, we have to mix things up so the same locations don't always light up
      randomSeed(analogRead(0));
      heatmap[random16(NUM_LEDS)] = random16(twink_min, twink_max);
     }
    }

    /* Boosting */
    for ( int k = 0 ; k < NUM_LEDS ; k++ ) {
      if (heatmap[k] > 0 && random8() < boost_thr) {
        heatmap[k] = qadd8(heatmap[k] , boost_rate);
      }
    }
    
    for ( int j = 0; j < NUM_LEDS; j++)
    {
      leds[j].setHSV(leds_h[j], 255, heatmap[j]);
    }
    offset++;

    /* Reset condition */
    if (offset == maxiter) {
      offset = 0;
      FastLED.clearData();

      seeds = random16(NUM_LEDS - seed_amt , NUM_LEDS);
      
      /* Reset heatmap */
      for (int i = 0; i< NUM_LEDS; i++)
      {
        heatmap[i] = 128;
        leds[i].setHSV(leds_h[i], 255, 0);
      }
      
      for ( int i = 0 ; i < seeds ; i++) 
      {
         random16_add_entropy( random(UCHAR_MAX));
         int pos = random16(NUM_LEDS);
         heatmap[pos] = random16(twink_min, twink_max);
      }

      currentPalette = generateRandomPalette(pal_cols);
      CRGB currLed;
      
      for ( int j = 0; j < NUM_LEDS; j++)
      {
        currLed = ColorFromPalette( currentPalette, random16(0, 15), heatmap[j], LINEARBLEND);
        leds[j] = currLed;
        leds_h[j] = rgb2hsv_approximate( currLed ).h;
      }
            
      cycle = 1;
    } 
  
}


/* Helper Methods */
void generateRandomFrame(CRGB frame[]) {    
  for (int i = 0; i < NUM_LEDS; i++) {
    random16_add_entropy( random(UCHAR_MAX));
    frame[i].setHSV(random(UCHAR_MAX), random(UCHAR_MAX), random(HSV_DEFAULT_VALUE));
  }
}

void generateRandomFrameFromPalette(CRGB frame[]) {  
  currentPalette = generateRandomPalette(pal_cols);
  
  uint8_t colIdx = 1;
  for (int i = 0; i < NUM_LEDS; i++) {
    frame[i] = ColorFromPalette( currentPalette, colIdx, HSV_DEFAULT_VALUE, LINEARBLEND);
    colIdx += 1;
  }
}


CRGBPalette16 generateRandomPalette(uint8_t colCount) {
    CRGBPalette16 palette;  
    CRGB colorSet[colCount];
  
    for (int j = 0; j < colCount; j++) {
        random16_add_entropy( random(UCHAR_MAX));
        colorSet[j] = CHSV(random16(0, UCHAR_MAX), 255, HSV_DEFAULT_VALUE);
    }
  
    for( int i = 0; i < 16; ++i) {
        palette[i] = colorSet[i % colCount];
    }

    return palette;  
}
