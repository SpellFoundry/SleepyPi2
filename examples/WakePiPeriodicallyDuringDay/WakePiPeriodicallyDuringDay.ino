// 
// Simple example showing how to set the RTC alarm pin to wake up the Arduino
// which in turn wakes up the Rpi. You can use this to do things with the Rpi 
// at regular intervals i.e. like taking a photograph.
//
// Note, in this example the Rpi is woken up for every 5 minutes. Once woken
// it is then shutdown after 1 minute. The system then goes into low power mode
// for the remaining 4 minutes before the Rpi is woken again and the cycle repeats.
//
// This is a different mode to the alarm clock, which wakes at a particular time.
// This mode is a repeating periodic time, waking the Arduino at fixed intervals.
//
// To test on the RPi without power cycling and using the Arduino IDE
// to view the debug messages, comment out these line (with a //):
//
// SleepyPi.enablePiPower(true);
// SleepyPi.piShutdown();      
// SleepyPi.enableExtPower(false);  
// 
// Also either fit the Power Jumper or enable
// self-power. http://spellfoundry.com/sleepy-pi/programming-arduino-ide/
// 

// **** INCLUDES *****
#include "SleepyPi2.h"
#include <TimeLib.h>
#include <LowPower.h>
#include <PCF8523.h>
#include <Wire.h>

const char *monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

const int LED_PIN = 13;

// Globals
// ++++++++++++++++++++ CHANGE ME ++++++++++++++++++
// This is the repeating time interval that the Rpi is powered up.
// NOTE: ACTUAL CYCLE TIME IS TIMER INTERVAL + HOW LONG RPI AWAKE
// Thus = 5 minutes = 4 mins + awake of 1 min
// .. Setup the Periodic Timer
// .. use either eTB_SECOND or eTB_MINUTE or eTB_HOUR
eTIMER_TIMEBASE  PeriodicTimer_Timebase     = eTB_MINUTE;   // e.g. Timebase set to seconds. Other options: eTB_MINUTE, eTB_HOUR
uint8_t          PeriodicTimer_Value        = 5;            // Timer Interval in units of Timebase e.g 5 minutes
// Time the Rpi stays awake for:
unsigned long    RPI_TIME_TO_STAY_AWAKE_MS  = 60000;       // in ms - so this is 60 seconds

// Define Daytime
uint8_t  WakeUp_StartHour       = 7;   // Hour in 24 hour clock
uint8_t  WakeUp_StartMinute     = 00;  // Minutes 
uint8_t  Bedtime_StartHour      = 20;  // Hour in 24 hour clock
uint8_t  Bedtime_StartMinute    = 00;  // Minutes 

// ++++++++++++++++++++ END CHANGE ME ++++++++++++++++++

tmElements_t tm;


void alarm_isr()
{
    // Just a handler for the alarm interrupt.
    // You could do something here
}

void setup()
{ 
  // Configure "Standard" LED pin
  pinMode(LED_PIN, OUTPUT);		
  digitalWrite(LED_PIN,LOW);		// Switch off LED

  // initialize serial communication: In Arduino IDE use "Serial Monitor"
  Serial.begin(9600);
  Serial.println("Starting, but I'm going to go to sleep for a while...");
  delay(50);
  SleepyPi.rtcInit(true);

  // Default the clock to the time this was compiled.
  // Comment out if the clock is set by other means
  // ...get the date and time the compiler was run
  if (getDate(__DATE__) && getTime(__TIME__)) {
      // and configure the RTC with this info
      SleepyPi.setTime(DateTime(F(__DATE__), F(__TIME__)));
  } 
  
  printTimeNow();   

  Serial.print("Periodic Interval Set for: ");
  Serial.print(PeriodicTimer_Value);
  switch(PeriodicTimer_Timebase)
  {
    case eTB_SECOND:
      Serial.print(" seconds");
      break;
    case eTB_MINUTE:
      Serial.print(" minutes");
      break;
    case eTB_HOUR:
      Serial.print(" hours");
    default:
        Serial.print(" unknown timebase");
        break;
  }
  Serial.print(" + ");
  Serial.print(RPI_TIME_TO_STAY_AWAKE_MS / 1000);
  Serial.println(" seconds");

}

void loop() 
{
    DateTime now;
  
    SleepyPi.rtcClearInterrupts();

    // Allow wake up alarm to trigger interrupt on falling edge.
    attachInterrupt(0, alarm_isr, FALLING);    // Alarm pin

    // Set the Periodic Timer
    SleepyPi.setTimer1(PeriodicTimer_Timebase, PeriodicTimer_Value);

    delay(500);

    // Enter power down state with ADC and BOD module disabled.
    // Wake up when wake up pin is low.
    SleepyPi.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 

    // Disable external pin interrupt on wake up pin.
    detachInterrupt(0);
    
    SleepyPi.ackTimer1();

    // Just a few things to show what's happening
    digitalWrite(LED_PIN,HIGH);    // Switch on LED
    Serial.println("I've Just woken up on a Periodic Timer!");
    // Print the time
    printTimeNow();   
    delay(50);
    digitalWrite(LED_PIN,LOW);    // Switch off LED  

    // Check whether this is 'daytime'. If it is, wakeup the Rpi if it isn't go back to sleep
    now = SleepyPi.readTime();
    if((now.hour() >= WakeUp_StartHour) && now.minute() >= WakeUp_StartMinute)
    {
      // Check whether it's also less than bedtime
      if((now.hour() <= Bedtime_StartHour) && now.minute() <= Bedtime_StartMinute)
      {
        // Waekup the Rpi
        SleepyPi.enablePiPower(true);   
            // Do something on the Rpi here (instead of the delay()
        // Example: Take a Picture
        // This is the time that the Rpi will be "awake" for
        delay(RPI_TIME_TO_STAY_AWAKE_MS);    

        // Start a shutdown
        SleepyPi.piShutdown();      
        SleepyPi.enableExtPower(false); 
        
      }
    }

   // End of main loop
}

// **********************************************************************
// 
//  - Helper routines
//
// **********************************************************************

void printTimeNow()
{
    // Read the time
    DateTime now = SleepyPi.readTime();
    
    // Print out the time
    Serial.print("Ok, Time = ");
    print2digits(now.hour());
    Serial.write(':');
    print2digits(now.minute());
    Serial.write(':');
    print2digits(now.second());
    Serial.print(", Date (D/M/Y) = ");
    Serial.print(now.day());
    Serial.write('/');
    Serial.print(now.month()); 
    Serial.write('/');
    Serial.print(now.year(), DEC);
    Serial.println();

    return;
}
bool getTime(const char *str)
{
  int Hour, Min, Sec;

  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  tm.Hour = Hour;
  tm.Minute = Min;
  tm.Second = Sec;
  return true;
}

bool getDate(const char *str)
{
  char Month[12];
  int Day, Year;
  uint8_t monthIndex;

  if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3) return false;
  for (monthIndex = 0; monthIndex < 12; monthIndex++) {
    if (strcmp(Month, monthName[monthIndex]) == 0) break;
  }
  if (monthIndex >= 12) return false;
  tm.Day = Day;
  tm.Month = monthIndex + 1;
  tm.Year = CalendarYrToTm(Year);
  return true;
}

void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}
