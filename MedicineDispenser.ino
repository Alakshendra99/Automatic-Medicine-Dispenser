#include <LiquidCrystal.h>
#include <RTClib.h>
#include <Servo.h>
/*------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*- Callibration-Parameters -*/
/*- Ultrasonic-Thershold -*/
const int Point = 8;
/*- Rotor-Moving-Time -*/
const int Hold = 131;
/*- Timeout-For-Opening -*/
const unsigned long Timeout = 120000;
/*- Timeout-For-Opening -*/
const unsigned long AlarmTime = 30000;
/*- Feeding-Time -*/
const int Time[7][3][2] = {
  /*- {HH,MM}-of-Three-Times -*/
  {{10,30},{14,00},{21,00}},  /*- Sunday    -*/
  {{10,30},{14,00},{21,00}},  /*- Monday    -*/
  {{10,30},{14,00},{21,00}},  /*- Tuesday   -*/
  {{10,30},{23,27},{21,00}},  /*- Wednesday -*/
  {{10,30},{23,42},{20,52}},  /*- Thursday  -*/
  {{00,51},{14,00},{21,00}},  /*- Friday    -*/
  {{10,30},{14,00},{21,00}}   /*- Saturday  -*/ 
};
/*------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*- Pin Connection -*/
/*-
 * D0 - RX
 * D1 - TX
 * D2 - D7 LCD
 * D3 - D6 LCD
 * D4 - D5 LCD
 * D5 - D4 LCD
 * D6 - E  LCD
 * D7 - RS LCD
 * D8 - Rotor
 * D9 - Servo
 * D10- TRIG Ultrasonic
 * D11- ECHO Ultrasonic
 * D12- Buzzer
 * D13- Signal
 * A0 - 
 * A1 -
 * A2 -
 * A3 -
 * A4 - SDA
 * A5 - SCL
-*/
/*------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*- Global-Issues -*/
const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
const int Rotor = 8;
const int Trig = 10;
const int Echo = 11;
const int Signal = 13;
const int Buzz = 12;

/*- Calculation -*/
long Duration;
long Distance;

Servo Motor;
RTC_DS1307 RTC;
char Day[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
/*------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void setup(){
  Serial.begin(57600);
  
  #ifndef ESP8266
    while (!Serial);
  #endif
  if (! RTC.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));   
  }
  
  lcd.begin(16, 2);
  pinMode(Echo,INPUT);
  pinMode(Buzz,OUTPUT);
  pinMode(Trig,OUTPUT);
  pinMode(Rotor,OUTPUT);
  pinMode(Signal,OUTPUT);

  /*- Ensuring-Starting-Safety -*/
  Motor.attach(9);
  Motor.write(0);
  lcd.print("Hello There!");
  digitalWrite(Buzz,LOW);
  digitalWrite(Signal,HIGH);

  Serial.println("Feeder Operation");
}
/*------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void loop(){
  delay(5000);
  Serial.print("Time ");
  DateTime now = RTC.now();
  
  char bufDate[] = "Today is DDD, MMM DD YYYY";
  Serial.println(now.toString(bufDate));
  char bufTime[] = "hh:mm";
  Serial.println(now.toString(bufTime));
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(Day[now.dayOfTheWeek()]);
  lcd.setCursor(0,1);
  lcd.print(bufTime);
  
  Serial.println("Checking - ");
  for(int i=0;i<3;i++){
    
    /*- Checking-The-Time -*/
    Serial.print(Time[now.dayOfTheWeek()][i][0]);
    Serial.print(":");
    Serial.print(Time[now.dayOfTheWeek()][i][1]);
    Serial.print(" With ");
    Serial.print(now.hour());
    Serial.print(":");
    Serial.println(now.minute());
    Serial.println();
    
    if(now.hour()==Time[now.dayOfTheWeek()][i][0]&&now.minute()==Time[now.dayOfTheWeek()][i][1]){
      /*- Alarming-About-Moving-Rotor -*/
      Serial.println("Time Has Arrived");
      digitalWrite(Signal,LOW);
      digitalWrite(Buzz,HIGH);
      delay(500);
      digitalWrite(Signal,HIGH);
      digitalWrite(Buzz,LOW);
      delay(500);
      digitalWrite(Signal,LOW);
      digitalWrite(Buzz,HIGH);
      delay(500);
      digitalWrite(Signal,HIGH);
      digitalWrite(Buzz,LOW);
      delay(500);
      
      /*- Moving-The-Rotor -*/
      Serial.println("Rotor Moving");
      digitalWrite(Rotor,HIGH);
      delay(Hold);
      digitalWrite(Rotor,LOW);
      bool Alarm = true;

      /*- Timeout-Entry -*/
      unsigned long EntryTime = millis();
      unsigned long NowTime = millis();
      
      while((NowTime-EntryTime)<Timeout){
        /*- Triggering-A-Ping -*/
        digitalWrite(Trig,LOW);
        delayMicroseconds(2);
        digitalWrite(Trig,HIGH);
        delayMicroseconds(10);
        digitalWrite(Trig,LOW);
        
        /*- Recieving-The-Ping -*/
        Duration = pulseIn(11,HIGH);
        Distance = Duration*0.034/2;
        Serial.println(Distance);

        if(Alarm&&((NowTime-EntryTime)<AlarmTime)){
          digitalWrite(Signal,LOW);
          digitalWrite(Buzz,HIGH);
          delay(500);
          digitalWrite(Signal,HIGH);
          digitalWrite(Buzz,LOW);
          delay(500);
        }

        /*- Openning-Gate -*/
        if(Distance<Point){
          Motor.write(90);
          Alarm = false;
          delay(30000);
        }else{
          Motor.write(0);
        }
        NowTime = millis();
      }

      /*- Timeout-Safety -*/ 
      digitalWrite(Signal,LOW);
      digitalWrite(Buzz,HIGH);
      delay(500);
      digitalWrite(Signal,HIGH);
      digitalWrite(Buzz,LOW);
      delay(500);
      digitalWrite(Signal,LOW);
      digitalWrite(Buzz,HIGH);
      delay(500);
      digitalWrite(Signal,HIGH);
      digitalWrite(Buzz,LOW);
      delay(500);
      Motor.write(0);
      Serial.println("Timeout\n");    
    }
  }
}
