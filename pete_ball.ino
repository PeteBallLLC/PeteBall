#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 6
#define LEFT_BUTTON 3
#define RIGHT_BUTTON 2

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
const int midPoint = LED_COUNT / 2;

//Debounce
volatile long leftLastTime = 0;
volatile long rightLastTime = 0;
const long debounceThreshold = ballSpeed + 50;

int currentIndex = midPoint; // where the ball is currently
int ballSpeed = 250; // delay in ms (changed by potentiometer)

// global colors
uint32_t BLACK;
uint32_t SALMON;
uint32_t RED;
uint32_t GREEN;
uint32_t BLUE;
uint32_t YELLOW;

void setup() {
  Serial.begin(9600);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  strip.setBrightness(64); //limit overall brightness

  // Button inputs 
  pinMode(LEFT_BUTTON, INPUT);
  pinMode(RIGHT_BUTTON, INPUT);

  // button interrupt definitions
  attachInterrupt(digitalPinToInterrupt(LEFT_BUTTON), leftPressed, RISING);
  attachInterrupt(digitalPinToInterrupt(RIGHT_BUTTON), rightPressed, RISING);

  // Color definitions
  BLACK = strip.Color(0, 0, 0);
  SALMON = strip.Color(40, 20, 10);
  RED = strip.Color(255, 0, 0);
  GREEN = strip.Color(0, 255, 0);
  BLUE = strip.Color(0, 0, 255);
  YELLOW = strip.Color(128, 128, 0);

  strip.setPixelColor(midPoint, SALMON); // set the middle LED to on

  // if analog input pin 1 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  randomSeed(analogRead(1));
  
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
  for (int i = leftEndPoint; i < rightEndPoint; i++) {
    if (i == currentIndex) {
      if (ballDir == right) {
        strip.setPixelColor(currentIndex, BLUE);
      } else {
        strip.setPixelColor(currentIndex, YELLOW);
      }
    } else {
      strip.setPixelColor(i, 0, 0, 0); 
    }
  }
}

// ISR for left Button
void leftPressed() {
  // debounce logic
  long currentTime = millis();
  if (currentTime - leftLastTime < debounceThreshold) {
    return;
  } else {
    leftLastTime = currentTime;
  }
  
  if ((currentIndex > midPoint) || ballDir == right) {
    // only allow capture attempt when ball is on your side and moving towards your end point
    return;
  } else {
    if ((currentIndex - 1) == leftEndPoint) {
      //keep this point lit
      ++leftEndPoint;
      ++currentIndex;
      ballDir = right;
    }
  }
}

// ISR for right Button
void rightPressed() {
  // debounce logic
  long currentTime = millis();
  if (currentTime - rightLastTime < debounceThreshold) {
    return;
  } else {
    rightLastTime = currentTime;
  }
  
  if ((currentIndex < midPoint) || ballDir == left) {
    // only allow capture attempt when ball is on your side and moving towards your end point
    return;
  } else {
    if ((currentIndex + 1) == rightEndPoint) {
      //keep this point lit
      --rightEndPoint;
      --currentIndex;
      ballDir = left;
    }
  }
}

