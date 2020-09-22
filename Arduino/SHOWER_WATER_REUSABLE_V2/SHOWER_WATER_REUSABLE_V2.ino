#include "LowPower.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_NeoPixel.h>

#include <AceButton.h>
using namespace ace_button;

#define DEFAULT_TEMP_MIN 20 
#define DEFAULT_TEMP_MAX 28

#define TIMEOUT_FINISH 10
#define TIMEOUT_ERROR 10

#define RING_LEDS_PIN 6

// Data wire is plugged into port 3 on the Arduino
#define ONE_WIRE_BUS 3

#define SOLENOID_VALVE_PIN 4

#define SWITCH_PIN 2

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(12, RING_LEDS_PIN, NEO_GRB + NEO_KHZ800);

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// One button wired to the pin at BUTTON_PIN. Automatically uses the default
// ButtonConfig. The alternative is to call the AceButton::init() method in
// setup() below.
AceButton button(SWITCH_PIN);

void handleEvent(AceButton*, uint8_t, uint8_t);

bool flash = false;
uint8_t timeout = 0;

uint8_t tempMax = DEFAULT_TEMP_MAX;

static uint8_t numberOfClick = 0;
static bool isClick = false;
static bool isFinish = false;

void myDelayFunction(uint16_t ms){
  uint16_t timeCounter = 0;
  
  while(timeCounter < (ms / 20)){
    timeCounter++;
    button.check();
    delay(20);  
  }
}

ISR(TIMER1_COMPA_vect){
   //interrupt commands for TIMER 1 here
   button.check();
}

float getTemperature(){
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  
  return sensors.getTempCByIndex(0);
}

void startFunction(){
while(1);
  float temp = getTemperature();
  
  Serial.print("Temperature : ");
  Serial.println(temp);

  if(temp > tempMax){
      
  }else if(temp < tempMax){
    
  }else if(temp == -127){
    Serial.println("Read temperature error !!!");
    //Turn off the solenoid valve
    digitalWrite(SOLENOID_VALVE_PIN, HIGH);
  }

  while(1);
}

void interrupt(){
  clickCounter++;
}

void abortFunction(){
    
}

void setup() {
  Serial.begin(115200);
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  // Start up the library
  sensors.begin();

  //Solenoid valve init
  pinMode(SOLENOID_VALVE_PIN, OUTPUT);

  // Switch init
  // Button uses the built-in pull up register.
  pinMode(SWITCH_PIN, INPUT_PULLUP);

  //attachInterrupt(0, interrupt, LOW);

  ButtonConfig* buttonConfig = button.getButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureSuppressClickBeforeDoubleClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureSuppressAfterClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureSuppressAfterDoubleClick);

  // TIMER 1 for interrupt frequency 50 Hz (20ms):
  cli(); // stop interrupts
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 0; // initialize counter value to 0
  // set compare match register for 50 Hz increments
  OCR1A = 39999; // = 16000000 / (8 * 50) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12, CS11 and CS10 bits for 8 prescaler
  TCCR1B |= (0 << CS12) | (1 << CS11) | (0 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei(); // allow interrupts

  Serial.println("Program start");
}

void loop() {
  // Should be called every 20ms or faster for the default debouncing time
  // of ~50ms.
  //button.check();
}

// The event handler for the button.
void handleEvent(AceButton* /* button */, uint8_t eventType,
    uint8_t /* buttonState */) {
  switch (eventType) {
    case AceButton::kEventClicked:
    case AceButton::kEventReleased:
      //Start the system
      Serial.println("Reusable cold water system start");
      startFunction();
      break;
    case AceButton::kEventDoubleClicked:
      //Abort the system
      Serial.println("Abort the reusable cold water sytem");
      abortFunction();
      break;
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    //bstrip.setBrightnessPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}
