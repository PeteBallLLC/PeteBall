#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 6

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
const int LED_COUNT = 21;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

enum BallDirection {
  right,
  left
};

BallDirection ballDir; // tracks which direction the "ball" (light) is moving currently

volatile int leftEndPoint = -1; //keeps track of how far left we should move the ball
volatile int rightEndPoint = LED_COUNT; //how far right
const int midPoint = 10;

int currentIndex = midPoint; // where the ball is currently
int ledPinsLength = 7;
int ballSpeed = 250; // delay in ms (changed by potentiometer)

uint32_t SALMON;

void setup() {
  Serial.begin(9600);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  // Color definitions
  SALMON = strip.Color(40, 20, 10);

  strip.setPixelColor(midPoint, SALMON); // set the middle LED to on

  // if analog input pin 0 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  randomSeed(analogRead(0));
  
  // determine starting direction randomly
  if (random(0, 100) < 50) {
    ballDir = left;
  } else {
    ballDir = right;
  }
  delay(1500); // for dramatic tension
}

void loop() {

  BallDirection currentDir = ballDir; // to reference against for detecting direction change
  ballSpeed = getSpeedFromKnob();
  Serial.println(currentIndex);
  moveBall(); //This will update currentIndex
  Serial.println(currentIndex);
  updateLeds();
  strip.show();
  if (currentDir == ballDir) {
    delay(ballSpeed); //only delay if we haven't just changed direction
  }
}

int getSpeedFromKnob() {
  return (int) map(analogRead(A0), 0, 1024, 15, 500);
}

void moveBall() {
  if (ballDir == right) {
    //if out of bounds
    if (++currentIndex == rightEndPoint) {
      ballDir = left;
      --currentIndex; //start moving back the other way
    }
    
  } else if (ballDir == left) {
    //if out of bounds
    if (--currentIndex == leftEndPoint) {
      ballDir = right;
      ++currentIndex; //start moving back the other way
    }
  }
}

void updateLeds() {
  // clear all LEDs first
  for (int i = 0; i < LED_COUNT; i++) {
    if (i == currentIndex) {
      strip.setPixelColor(currentIndex, SALMON);   
    } else {
      strip.setPixelColor(i, 0, 0, 0); 
    }
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, SALMON);
    }
    strip.show();
    delay(wait);
  }
}

