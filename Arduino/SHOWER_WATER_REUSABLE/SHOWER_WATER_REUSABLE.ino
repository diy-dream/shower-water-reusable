#include "LowPower.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_NeoPixel.h>
#include <SimpleCLI.h>

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

// Create CLI Object
SimpleCLI cli;

// Commands
Command cmdSetMaxTemp;
Command cmdSetMinTemp;
Command cmdGetTemp;
Command cmdHelp;

bool flash = false;
uint8_t timeout_finish = 0;
uint8_t timeout_error = 0;

uint8_t tempMax = DEFAULT_TEMP_MAX;

float getTemperature(){
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  
  return sensors.getTempCByIndex(0);
}

void wakeUp()
{ 
    bool isFinish = false;
    timeout_finish = 0;
    timeout_error = 0;
    uint8_t counter = 0;
    
    // Just a handler for the pin interrupt.
    
    // Disable external pin interrupt on wake up pin.
    detachInterrupt(0);

    //while(!isFinish || timeout_error < TIMEOUT_ERROR){
    while(timeout_error < TIMEOUT_ERROR){

      float temp = getTemperature();
      
      Serial.print("Temperature : ");
      Serial.println(temp);

      if(temp == -127 && counter < 3){
        counter++;
      } else if(temp == -127 && counter == 3){
        //Error temperature sensor

        //Turn off the solenoid valve
        digitalWrite(SOLENOID_VALVE_PIN, LOW);
        Serial.println("Error temperature sensor");
        
        timeout_finish = 0;
        while(timeout_finish < TIMEOUT_FINISH){
            if(flash){
              flash = false;
              colorWipe(strip.Color(240, 195, 0), 0); // Orange
            }else{
              flash = true;
              colorWipe(0, 0); // Turn off the ring leds off
            }
            timeout_finish++;
            delay(400);
        }
        colorWipe(0, 0); // Turn off the ring leds off
        break;
      }
    
      if(temp > tempMax){
        //Turn off the solenoid valve
        Serial.println("The temperature is higher than TEMP_MAX");

        digitalWrite(SOLENOID_VALVE_PIN, LOW);
        colorWipe(strip.Color(0, 255, 0), 50); // Green
        while(timeout_finish < TIMEOUT_FINISH){
          if(flash){
            flash = false;
            colorWipe(strip.Color(0, 255, 0), 0); // Green
          }else{
            flash = true;
            colorWipe(0, 0); // Turn off the ring leds off
          }
          timeout_finish++;
          delay(400);
        }

        colorWipe(0, 0); // Turn off the ring leds off
        isFinish = true;
      }else{
        Serial.println("The temperature is lower than TEMP_MAX");
        
        //Turn on the solenoid valve
        digitalWrite(SOLENOID_VALVE_PIN, HIGH);
        
        colorWipe(strip.Color(255, 0, 0), 50); // Red
        colorWipe(0, 50); // Red
      }
      
      timeout_error++;
      Serial.print("timeout_error = ");
      Serial.println(timeout_error);
    }

    if(timeout_error == TIMEOUT_ERROR){
      //Turn off the solenoid valve
      digitalWrite(SOLENOID_VALVE_PIN, LOW);
      Serial.println("Timeout to get a good temperature");
      
      timeout_finish = 0;
      while(timeout_finish < TIMEOUT_FINISH){
          if(flash){
            flash = false;
            colorWipe(strip.Color(44, 117, 255), 0); // Orange
          }else{
            flash = true;
            colorWipe(0, 0); // Turn off the ring leds off
          }
          timeout_finish++;
          delay(400);
      }
      colorWipe(0, 0); // Turn off the ring leds off
    }
    
    // WAKE-UP configuration
    // Allow wake up pin to trigger interrupt on low.
    attachInterrupt(0, wakeUp, LOW);

    // Enter power down state with ADC and BOD module disabled.
    // Wake up when wake up pin is low.
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
}

void setup() {
  Serial.begin(115200);

  //CLI init
  cmdSetMaxTemp = cli.addCmd("temp");
  cmdSetMaxTemp.addArg("max");
  cmdSetMaxTemp.setDescription("Set the max temperature");

  cmdSetMinTemp = cli.addCmd("temp");
  cmdSetMinTemp.addArg("min");
  cmdSetMinTemp.setDescription("Set the min temperature");

  cmdGetTemp = cli.addCmd("getTemp");
  cmdGetTemp.setDescription("Get the actual temperature");

  cmdHelp = cli.addCommand("help");
  cmdHelp.setDescription(" Get help!");
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  // Start up the library
  sensors.begin();

  //Solenoid valve init
  pinMode(SOLENOID_VALVE_PIN, OUTPUT);

  //Switch init
  pinMode(SWITCH_PIN, INPUT);

  // WAKE-UP configuration
  // Allow wake up pin to trigger interrupt on low.
  attachInterrupt(0, wakeUp, LOW);

  // Enter power down state with ADC and BOD module disabled.
  // Wake up when wake up pin is low.
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
}

void loop() {
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');

        if (input.length() > 0) {
            Serial.print("# ");
            Serial.println(input);

            cli.parse(input);
        }
    }

    if (cli.available()) {
        Command c = cli.getCmd();

        int argNum = c.countArgs();

        Serial.print("> ");
        Serial.print(c.getName());
        Serial.print(' ');

        for (int i = 0; i<argNum; ++i) {
            Argument arg = c.getArgument(i);
            // if(arg.isSet()) {
            Serial.print(arg.toString());
            Serial.print(' ');
            // }
        }

        Serial.println();

        if (c == cmdSetMaxTemp) {
            Serial.println("The new maximum temperature = " + c.getArgument("max").getValue());
            tempMax = c.getArgument("max").getValue().toInt();
        } else if (c == cmdSetMinTemp) {
            Serial.println("cmdGetTemp new minimum temperature = " + c.getArgument("min").getValue());
        } else if (c == cmdGetTemp) {
            Serial.print("The temperature = ");        
            Serial.println(getTemperature());        
        } else if (c == cmdHelp) {
            Serial.println("Help:");
            Serial.println(cli.toString());
        }
    }

    if (cli.errored()) {
        CommandError cmdError = cli.getError();

        Serial.print("ERROR: ");
        Serial.println(cmdError.toString());

        if (cmdError.hasCommand()) {
            Serial.print("Did you mean \"");
            Serial.print(cmdError.getCommand().toString());
            Serial.println("\"?");
        }
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
