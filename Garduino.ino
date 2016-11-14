#include <ThingSpeak.h>
#include <dht.h>
#include <SPI.h>
#include <Ethernet.h>


/* Garduino by Daniel Franulovic
 *  From tutorial examples on the web, free use
 *  - Analog signal read for humidity
 *  - Digital led gauge for output of analog value (5 leds, from green to red, depending on the level of signal from 0 super wet to 1024 super dry)
 *  - Air humidity DHT11 sensor on PWM pin that prints to monitor (for future saving on web server, wi-fi module pending)
 *  - Light sensor on analog with min/max adjustments (for future saving on cloud server, wi-fi module pending)
 *  - Water pump control based on soil humidity value
 *  - ThingSpeak integration for cloud upload of values
*/

// Ethernet declarations
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
EthernetClient client;

// ThingSpeak
unsigned long myChannelNumber = 174024;
const char * myWriteAPIKey = "I9651R8WWE30XEZW";
int timer = 200;

// leds for the digital output
int led1 = 3;
int led2 = 4;
int led3 = 5;
int led4 = 6;
int led5 = 7;

// Declarations of variables for Light Sensor
int photoRPin = A2;      // Using A2 
int minLight;           // Used to calibrate the readings
int maxLight;           // Used to calibrate the readings
int lightLevel;
int adjustedLightLevel;

// Variables for Humidity Sensor
int humidityRPin = A0;    // Using A0

// Variables for DHT (air humidity and temperature) Sensor
dht DHT;              // DHT variable
#define DHT11_PIN 8   // the PWM pin the DHT11 is attached to

// Lamp control
#define RELAY_ON 0
#define RELAY_OFF 1
#define lampPin A5   // Using analog pin for lamp relay (digital pins are full)
int lightState = RELAY_OFF; // Lamp off

// Code for water pump
#define WATER_ON 0
#define WATER_OFF 1
#define waterPin 1    // Digital pin for water pump
int pumpSafety = WATER_ON;   // Safety mode to disable pump if humidity detector fails
int pumpCount = 0;
int pumpState = WATER_OFF;
 
// the setup routine runs once when you press reset:
void setup() {

  // declare digital gauge leds to be outputs
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  pinMode(led5, OUTPUT);

  //Setup the starting light level limits (for Light Sensor)
  lightLevel=analogRead(photoRPin);
  minLight=lightLevel-20;
  maxLight=lightLevel;


  // guarantee that lamp is off
  digitalWrite(lampPin, RELAY_OFF);
  // declare lamp pin to be output
  pinMode(lampPin, OUTPUT);

  // guarantee that pump is off
  digitalWrite(waterPin, WATER_OFF);
  // declare water pin to be output
  pinMode(waterPin, OUTPUT);

  // Ethernet
  Ethernet.begin(mac);

  // ThingSpeak
  ThingSpeak.begin(client);  

  // Waiting for sensors to stabilize
  delay(1000);
    
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {

  // read the input for soil humidity value:
  int humidity = analogRead(humidityRPin);


  // print out the value you read for debug purposes:
  // Serial.println(humidity);

  // DHT11 section of the code
  // read the input for air humidity and temperature
  int chk = DHT.read11(DHT11_PIN);

   // If you want to use serial monitor for DHT11 values, uncomment this section
  /*Serial.print("Temperature = ");
  Serial.println(DHT.temperature);
  Serial.print("Humidity = ");
  Serial.println(DHT.humidity);
  
  
  Serial.print("Pump State = ");
  Serial.println(pumpState);
  Serial.print("Pump Safety = ");
  Serial.println(pumpSafety);
  Serial.print("Pump Count = ");
  Serial.println(pumpCount);
  */


  if (pumpCount > 10) pumpSafety = WATER_OFF;   // Disable pump if it has been running for over 10 seconds
  
  if (humidity >= 900) // totally dry
  { 
    digitalWrite(led1, HIGH);
    digitalWrite(led2, HIGH);
    digitalWrite(led3, HIGH);
    digitalWrite(led4, HIGH);
    digitalWrite(led5, HIGH);
    if (pumpSafety == WATER_ON)  
      {
        digitalWrite (waterPin, WATER_ON);
        pumpState = WATER_ON;
        delay(1000);  // 1 second for pump to run and humidify soil
        timer = timer + 10;
        pumpCount++;
      }
    else
      {
        digitalWrite (waterPin, WATER_OFF);
      }
  }
  else if (humidity >= 600  && humidity < 900) // very dry
  {
    digitalWrite(led1, HIGH);
    digitalWrite(led2, HIGH);
    digitalWrite(led3, HIGH);
    digitalWrite(led4, HIGH);
    digitalWrite(led5, LOW);
    if (pumpSafety == WATER_ON)  
      {
        digitalWrite (waterPin, WATER_ON);
        pumpState = WATER_ON;
        delay(1000);  // 1 second for pump to run and humidify soil
        timer = timer + 10;
        pumpCount++;
      }
    else
      {
        digitalWrite (waterPin, WATER_OFF);
      }
  }  
  else if (humidity >= 300 && humidity < 600) // dry
  {
    digitalWrite(led1, HIGH);
    digitalWrite(led2, HIGH);
    digitalWrite(led3, HIGH);
    digitalWrite(led4, LOW);
    digitalWrite(led5, LOW);
    if (pumpSafety == WATER_ON)  
      {
        digitalWrite (waterPin, WATER_ON);
        pumpState = WATER_ON;
        delay(1000);  // 1 second for pump to run and humidify soil
        timer = timer + 10;
        pumpCount++;
      }
    else
      {
        digitalWrite (waterPin, WATER_OFF);
      }
  }    
  else if (humidity >= 100 && humidity < 200) // ok
  {
    digitalWrite(led1, HIGH);
    digitalWrite(led2, HIGH);
    digitalWrite(led3, LOW);
    digitalWrite(led4, LOW);
    digitalWrite(led5, LOW);
  
    digitalWrite (waterPin, WATER_OFF); 
    pumpState = WATER_OFF;
    pumpCount = 0;  
  }
  else if (humidity >= 40 && humidity < 100) // wet
  {
    digitalWrite(led1, HIGH);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    digitalWrite(led4, LOW);
    digitalWrite(led5, LOW);
    digitalWrite (waterPin, WATER_OFF); 
    pumpState = WATER_OFF;
    pumpCount = 0;  
  }
  else if (humidity < 40) // drowning
  {
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    digitalWrite(led4, LOW);
    digitalWrite(led5, LOW);
    digitalWrite (waterPin, WATER_OFF); 
    pumpState = WATER_OFF;
    pumpCount = 0;
  }

  //Serial.println(lightState); //debug 

  // Light sensor code below
  //auto-adjust the minimum and maximum limits in real time
  lightLevel=analogRead(photoRPin);
  if(minLight>lightLevel){
    minLight=lightLevel;
  }
  if(maxLight<lightLevel){
    maxLight=lightLevel;
  }
 
  //Adjust the light level to produce a result between 0 and 100.
  adjustedLightLevel = map(lightLevel, minLight, maxLight, 0, 100); 


  if (adjustedLightLevel <= 20 && lightState == RELAY_OFF)
  {
    digitalWrite (lampPin, RELAY_ON);
    lightState = RELAY_ON;
  }
  else if (adjustedLightLevel > 20 && lightState == RELAY_ON)
  {
    digitalWrite (lampPin, RELAY_OFF); //only turn lamp off if it is on  
    lightState = RELAY_OFF;       
  }
 
  //Send the adjusted Light level result to Serial port (processing)
  
  Serial.print("Light Level = ");
  Serial.println(adjustedLightLevel);
  
  // End of light sensor code

  // Write to ThingSpeak
  if (timer >= 200)
  {
    ThingSpeak.setField(1, humidity);
    ThingSpeak.setField(2, (float)DHT.humidity);
    ThingSpeak.setField(3, (float)DHT.temperature);
    ThingSpeak.setField(4, adjustedLightLevel);
    ThingSpeak.setField(5, pumpState);
    ThingSpeak.setField(6, pumpSafety);    
    ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    
  Serial.println("ThingSpeak writes: ");
  Serial.print("Humidity = ");
  Serial.println(humidity);
  Serial.print("Air Humidity = ");
  Serial.println(DHT.humidity);
  Serial.print("Air Temperature = ");
  Serial.println(DHT.temperature);
  Serial.print("Light Level = ");
  Serial.println(adjustedLightLevel);
  Serial.print("Pump State = ");
  Serial.println(pumpState);
  Serial.print("Pump Safety = ");
  Serial.println(pumpSafety);
  Serial.print("BTW, timer = ");
  Serial.println(timer);
  Serial.print("BTW, pump count = ");
  Serial.println(pumpCount);
  timer = 0;
  }
  delay(100);
  timer++;
      
}


