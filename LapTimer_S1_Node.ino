/*  THIS CODE IS FOR FSAE ULtrasonic LapTimer
 *  
 *        created by dhupee_haj 
 *        
 *  This Code is originally intented for helping my time calculate time on FSAE car testing
 *  but I decided that maybe its a good idea to share this knowledge to everybody.
 * 
 *  This project use HC-SR04 Ultrasonic Sensor for detecting car, 2 push button, one I2C 16x2 LCD Screen
 *  NRF24L01+ for wireless communication and of course, Arduino.....i use UNO for this.
 *  
 *  This project is not limited to FSAE car, you can use this for another froject that need lap timing
 *  also its not limited to 2 Arduinos, since NRF24L01+ is able to talk with 6 other NRF24L01+ and have long range
 *  
 *  I will add a license to this project in another time
*/



#include <SPI.h>                //include SPI library
#include <Wire.h>               //include Wire library
#include <LiquidCrystal_I2C.h>  //include LiquidCrystal_I2C library from fmalpartida
#include <NewPing.h>            //include NewPing library
#include <NRFLite.h>            //include NRFLite library
//#include <DHT.h>                //include DHT Libraries from Adafruit


//#define DHTPIN 6              // DHT-22 Output Pin connection
//#define DHTTYPE DHT11         // DHT Type
#define TRIGGER_PIN  7         //make trigger pin and echo pin parallel
#define ECHO_PIN     7         //make trigger pin and echo pin parallel
#define MAX_DISTANCE 400        //max distance of the sensor is 400 cm
//#define LED          3
#define STARTSTOP    A0         //define start/stop button
#define RESET        A1         //define reset button

LiquidCrystal_I2C lcd(0x3F ,2,1,0,4,5,6,7,3, POSITIVE); //check your i2c lcd address before connect

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); 

const static uint8_t RADIO_ID = 11111;              //ID of the Radio
const static uint8_t SECTOR1_RADIO_ID = 00000;      //ID of the Sector1 Radio
//const static uint8_t SECTOR2_RADIO_ID = ??;      //ID of the Sector2 Radio (if you use it)
//const static uint8_t SECTOR3_RADIO_ID = ??;      //ID of the Sector3 Radio (if you use it)
//const static uint8_t SECTOR4_RADIO_ID = ??;      //ID of the Sector4 Radio (if you use it)
const static uint8_t PIN_RADIO_CE = 9;          //CE connected to pin 9 arduino
const static uint8_t PIN_RADIO_CSN = 10;        //CSN connected to pin 9 arduino
/*MOSI  -> 11 (Hardware SPI MOSI)
**MISO  -> 12 (Hardware SPI MISO)
**SCK   -> 13 (Hardware SPI SCK)
**FYI
*/

NRFLite _radio;
uint32_t _FLtrigger;
uint32_t _Sector1;
//uint32_t _Sector2;
//uint32_t _Sector3;
//uint32_t _Sector4;


bool  FLtrigger = false;   //FL = Finish Line
bool  sector1   = false;
//bool  sector2   = false; //just use if you need multisector laptimer
//bool  sector3   = false; //just use if you need multisector laptimer
//bool  sector4   = false; //just use if you need multisector laptimer 

//float hum;    // Stores humidity value in percent
//float temp;   // Stores temperature value in Celcius
float duration; // Stores HC-SR04 pulse duration value
float distance; // Stores calculated distance in cm
//float soundsp;  // Stores calculated speed of sound in M/S
//float soundcm;  // Stores calculated speed of sound in cm/ms
int   iterations = 7;

unsigned long start, finish, dataStopWatch;

int  i=0;
int  fPause = 0;
long lastButton = 0; 
long delayAntiBouncing = 50; 
long dataPause = 0;

void setup(){
  Serial.begin (115200);
  //dht.begin();        //you can ignore this if not using DHT
  pinMode(A0,INPUT);
  pinMode(A1,INPUT);
  digitalWrite(A0,1);
  digitalWrite(A1,1);
  lcd.begin(16, 2);

  lcd.setCursor(0, 0);        // Opening screen :3
  lcd.print("BIMASAKTI UGM");
  lcd.setCursor(0, 1); 
  lcd.print("  LAP TIMER  ");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("   Made by   ");
  lcd.setCursor(0, 1);
  lcd.print(" dhupee_haj  "); //THAT'S ME !!
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  Press Button");
  lcd.setCursor(0, 1); 
  lcd.print("  Start / Stop");

  if (!_radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN))
    {
        Serial.println("Cannot communicate with radio");
        while (1); // Wait here forever.
    }

}

void loop(){

   //Uncomment this code if using DHT compensator
  /*delay(2000);                     // Delay so DHT-22 sensor can stabalize
    hum = dht.readHumidity();     // Get Humidity value
    temp= dht.readTemperature();  // Get Temperature value
    soundsp = 331.4 + (0.606 * temp) + (0.0124 * hum);  // Calculate the Speed of Sound in M/S
    soundcm = soundsp / 10000;                          // Convert to cm/ms
    duration = sonar.ping_median(iterations);
    distance = (duration / 2) * soundcm;                  // Calculate the distance
  */

   //use this if not use DHT compensator
   duration = sonar.ping_median(iterations); 
   distance = (duration / 2) * 0.0343;      
while (_radio.hasData()){
     _radio.readData(&_FLtrigger);
 if ( _FLtrigger == 1|| digitalRead(A0)==0) { //if ultrasonic trigger object or start/stop button pressed then STOPWATCH START!!
   if ((millis() - lastButton) > delayAntiBouncing){
      if (i==0){
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Start Timer");
          start = millis();
          fPause = 0;
          FLtrigger = true;
          //give data message to another MCU with NRF24 for start stopwatch
        }
       else if (i==1 || distance <=150 && distance >= 2 && FLtrigger == true){
          lcd.setCursor(0, 0);
          lcd.print("Stop Timer ");
          dataPause = dataStopWatch;
          fPause = 1;
          _Sector1 = 1;
          _radio.send(1, &_Sector1, sizeof(_Sector1)); 
          //give data message to another MCU with NRF24 for stop stopwatch
        }
       i =!i;
      }
      lastButton = millis();
  }

   else if (digitalRead(A1)==0 && fPause == 1){
      dataStopWatch = 0;
      dataPause = 0;
      FLtrigger = false; 
      lcd.clear();
      lcd.print("Reset Stopwatch");
      lcd.setCursor(0, 1); 
      lcd.print("0:0:0.0");
      //give data message to another MCU to restart stopwatch  
      delay(2000);
      lcd.clear();
      lcd.print("  Press Button");
      lcd.setCursor(0, 1); 
      lcd.print("  Start / Stop");
 }
}

if (i==1){
      finish = millis(); 
      float hour, minute, second, milisecond;
      unsigned long over;

      // MATH time!!!
      dataStopWatch = finish - start;
      dataStopWatch = dataPause + dataStopWatch;

      hour = int(dataStopWatch / 3600000);
      over = dataStopWatch % 3600000;
      minute = int(over / 60000);
      over = over % 60000;
      second = int(over / 1000);
      milisecond = over % 1000;

      lcd.setCursor(0, 1);
      lcd.print(hour, 0); 
      lcd.print(":"); 
      lcd.print(minute, 0);
      lcd.print(":");
      lcd.print(second, 0);
      lcd.print(".");
      if (hour < 10){
          lcd.print(milisecond, 0);
          lcd.print("   ");
       }
    }
}