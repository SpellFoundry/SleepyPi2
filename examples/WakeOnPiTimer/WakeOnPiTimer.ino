//
// Sleepy Pi to wake on button press or RTC timer interrupt, wake up pi and wait
// pi should initiate shutdown and set RTC timer interrupt for next wakeup
// sleepy PIN8 (B0) is assumed jumpered to pi's GPIO #17
// see rc.local

// **** INCLUDES *****
#include "SleepyPi2.h"
#include <TimeLib.h>
#include <LowPower.h>
#include <Wire.h>
#include <PCF8523.h>

// **** HW pins *****
const int ALARM_PIN      = 0;
const int BUTTON_PIN     = 1;
const int PI_IS_RUNNING  = 7;
const int MODE_PIN       = 8;
const int LED_PIN        = 13;

// **** DEFAULTS *****
eTIMER_TIMEBASE  timer_timebase     = eTB_HOUR;   // eTB_SECOND, eTB_MINUTE, eTB_HOUR
uint8_t          timer_value        = 1;

// **** STATE *****
typedef enum {
  wake_unknown,
  wake_power,
  wake_alarm,
  wake_button
} wakeOn;

wakeOn wu_state = wake_unknown;

// **** INTERRUPTS *****

// button
void button_isr()
{
  wu_state = wake_button;
}

// alarm
void alarm_isr()
{
  wu_state = wake_alarm;
}

// I2C
void receiveEvent(int howMany)
{
  Serial.println("i2c message received");

  uint8_t hour = Wire.read();
  uint8_t minute = 0;

  if (255 == hour)
  {
    Serial.println("ignoring 255 val - i2cdetect");    
    return;
  }

  if (Wire.available() > 0) {
    minute = Wire.read();
  }

  Serial.print("timer values: ");
  Serial.print(hour);
  Serial.print(":");
  Serial.println(minute);

  if (0 == hour)
  {
    timer_timebase = eTB_MINUTE;
    timer_value = minute;
  }
  else
  {
    timer_timebase = eTB_HOUR;
    timer_value = hour;
  }
}

void setup()
{
  wu_state = wake_power;

  // LED setup
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // use pin 8 to put pi in special mode
  pinMode(MODE_PIN, OUTPUT);
  digitalWrite(MODE_PIN, LOW);

  // use I2C to communicate pi ==> sleepy
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  delay(50);

  // Allow wake up triggered by button press
  attachInterrupt(BUTTON_PIN, button_isr, LOW);
  
  SleepyPi.rtcInit(true);

  // initialize serial communication: In Arduino IDE use "Serial Monitor"
  Serial.begin(9600);
  delay(50);
}

void pi_on()
{
  SleepyPi.enableExtPower(true);
  SleepyPi.enablePiPower(true);
  digitalWrite(LED_PIN, HIGH);
}

void pi_off()
{
  SleepyPi.piShutdown();
  SleepyPi.enableExtPower(false);
  digitalWrite(LED_PIN, LOW);
}

bool wait_pi_state(bool state, unsigned long s_timeout)
{
  unsigned long start = millis();
  bool ledState = HIGH;
  int blink_delay = s_timeout < 100 ? 250 : 1000;

  while (millis() - start < s_timeout * 1000)
  {
    if (state == SleepyPi.checkPiStatus(false))
    {      
      return true;
    }
    delay(blink_delay);
    digitalWrite(LED_PIN, ledState);
    ledState = !ledState;
  }
  return false;
}

void loop() 
{
  SleepyPi.rtcClearInterrupts();
  SleepyPi.enableWakeupAlarm(true);  

  Serial.println('pi wakeup...');
  pi_on();  
  
  bool state = wait_pi_state(true, 20);
  Serial.println(state ? 'pi ON' : 'pi ON failed!');

  digitalWrite(LED_PIN, state);
  delay(3000);
  state = wait_pi_state(false, 3600);  // 1 hour
  Serial.println(state ? 'pi OFF' : 'pi OFF failed!');
  pi_off();

  attachInterrupt(BUTTON_PIN, button_isr, LOW);
  attachInterrupt(ALARM_PIN, alarm_isr, FALLING);
  SleepyPi.setTimer1(timer_timebase, timer_value);
  delay(100);

  SleepyPi.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
  detachInterrupt(ALARM_PIN);
  detachInterrupt(BUTTON_PIN);
  SleepyPi.ackTimer1();

  digitalWrite(MODE_PIN, wu_state == wake_button ? HIGH : LOW);
}
