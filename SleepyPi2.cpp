/*
 * SleepyPi2.cpp - library for Sleepy Pi Board
  
  Copyright (c) Jon Watkins 2016
  http://spellfoundry.com

  This is a library of functions for use with the Sleepy Pi2 Power
  Management board for Raspberry Pi. 

  NOTE
  =======
  This library is dependent on the following libraries which must also
  be present in the libraries directory:

  - lowpower (http://www.rocketscream.com/blog/2011/07/04/lightweight-low-power-arduino-library/)
  			 (https://github.com/rocketscream/Low-Power)
  
  - pcf8523  (https://github.com/SpellFoundry/pcf8523.git)


  License
  =======
  The library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Releases
  ========
  V1_0 - 23 Aug 2013	- Initial release
  V2_0 - 27 April 2016	- Initial Modification to run Sleepy Pi 2	

 */

#include "SleepyPi2.h"

#define ENABLE_PI_PWR_PIN	16				// PC2 - O/P take high to enable the RaspPi - Active High
#define ENABLE_EXT_PWR_PIN	4				// PD4 - O/P take high to enable the External Supplies
#define CMD_PI_TO_SHDWN_PIN	17				// PC3 - 0/P Handshake to request the Pi to shutdown - Active high
#define PI_IS_RUNNING		7				// PD7 - I/P Handshake to show that the Pi is running - Active High
#define V_SUPPLY_PIN		A6				// I/P - A/I Supply monitoring pin
#define I_MONITOR_PIN		A7				// I/P - A/I Current monitoring pin	  
#define POWER_BUTTON_PIN	3				// PD3 - I/P User Power-on Button (INT1) - Active Low
#define ALARM_PIN			2				// PD2 - I/P Pin that pulses when the alarm has expired (INT0) - Active Low


// Constructors
SleepyPiClass::SleepyPiClass()
{
//	RTCConfig_t rtc_config; 

	// Reset flags
	simulationMode = false;
	simPiOn		   = false;
	pi_running	   = false;
	power_on	   = false;
	ext_power_on   = false;

	// **** Configure Power supplies ***
	// ...Configure Pi Power
	pinMode(ENABLE_PI_PWR_PIN, OUTPUT);		
	SleepyPiClass::enablePiPower(false);	// ***** RasPi is Off at Startup ***** //

	// ...Configure Ext Expansion Power
	pinMode(ENABLE_EXT_PWR_PIN, OUTPUT);		
	SleepyPiClass::enableExtPower(false);	// ***** Expansion Power is Off at Startup ***** //
											// Change if RasPi is to be powered on at boot
	// **** Configure I/O *****
	// ...Configure Pi Shutdown handshake to PI
	pinMode(CMD_PI_TO_SHDWN_PIN, OUTPUT);     
	digitalWrite(CMD_PI_TO_SHDWN_PIN,LOW);	// Don't command to shutdown
  
	// ...Configure Pi Shutdown handshake from PI - goes high when Pi is running
	pinMode(PI_IS_RUNNING, INPUT);    

	// ...Initialize the button pin as a input: also can be used as an interrupt INT1
	pinMode(POWER_BUTTON_PIN, INPUT);
 
	// ...Initialize the alarm as a input: also can be used as an interrupt INT0
	pinMode(ALARM_PIN, INPUT);

#ifdef DEBUG_MESSAGES
	int handShake;
	// initialize serial communication:
	Serial.begin(9600);

	// Display Status
	//read the state of the Pi
	handShake = digitalRead(PI_IS_RUNNING);
	if(handShake > 0){
	 Serial.println("Handshake I/P high");    
	}
	else {
	 Serial.println("Handshake I/P low");        
	}
	Serial.println("Pi Shutdown O/P low");
	Serial.println("Pi Power O/P low");

#endif
  
}
  
// PUBLIC FUNCTIONS
/* **************************************************+

	--enableExtPower

	Switch on / off the power to the external 
	expansion pins.

+****************************************************/
void SleepyPiClass::enableExtPower(bool enable)
{
	if(simulationMode == true){
		if(enable == true){
			// Turn on the External Expansion power
			// digitalWrite(ENABLE_EXT_PWR_PIN,HIGH);
			ext_power_on = true;			
		}
		else {
			// Turn off the External Expansion power
			// digitalWrite(ENABLE_EXT_PWR_PIN,LOW);
			ext_power_on = false;			
		}
	}
	else {
		if(enable == true){
			// Turn on the External Expansion power
			digitalWrite(ENABLE_EXT_PWR_PIN,HIGH);
			ext_power_on = true;
		}
		else {
			// Turn off the External Expansion power
			digitalWrite(ENABLE_EXT_PWR_PIN,LOW);
			ext_power_on = false;
		}	
	}
	return;
}
/* **************************************************+

	--enablePiPower

	Switch on / off the power to the Raspberry Pi.

+****************************************************/
void SleepyPiClass::enablePiPower(bool enable)
{
	if(simulationMode == true){
		if(enable == true){
			// Turn on the Pi
			// digitalWrite(ENABLE_PI_PWR_PIN,HIGH);
			power_on = true;
		}
		else {
			// Turn off the Pi
			// digitalWrite(ENABLE_PI_PWR_PIN,LOW);
			power_on = false;
		}
	}
	else {
		if(enable == true){
			// Turn on the Pi
			digitalWrite(ENABLE_PI_PWR_PIN,HIGH);
			power_on = true;
		}
		else {
			// Turn off the Pi
			digitalWrite(ENABLE_PI_PWR_PIN,LOW);
			power_on = false;
		}
	}
	return;
}

/* **************************************************+

	--enableWakeupAlarm

	Setup and enable the RTC Alarm. This will output
	a pulse on the /INT pin of the RTC which is recieved
	as a low going pulse on the INT0 pin of the ATMEGA

+****************************************************/
void SleepyPiClass::enableWakeupAlarm(bool enable)
{
	enableAlarm(enable);
	return;
}
/* **************************************************+

	--StartPiShutdown

	Set the handshake lines so that the Raspberry Pi
	is given notice to begin shutting down.

+****************************************************/
void SleepyPiClass::startPiShutdown(void)
{
	if(simulationMode == true){
		// Serial.println("StartPiShutdown()");
		power_on = false;
		ext_power_on = false;
	}
	else {
		// Command the Sleepy Pi to shutdown
		digitalWrite(CMD_PI_TO_SHDWN_PIN,HIGH);	
	}

	return;
}

/* **************************************************+

	--checkPiStatus

	Monitor the handshake lines and determine whether
	the Raspberry Pi is running or not.

	Option to Force a shutdown i.e. remove power to
	the Raspberry Pi if it is not handshaking.

+****************************************************/

bool SleepyPiClass::checkPiStatus(bool forceShutdownIfNotRunning)
{
	int	handShake;
	
	if(simulationMode == true){	
		handShake = power_on;
		pi_running = true;
	}
	else {
		handShake = digitalRead(PI_IS_RUNNING);		
	}	

	if(handShake > 0) {
		// RasPi is still running
		pi_running = true;
		return true;
	}
	else{
		// Pi not handshaking - either booting or manually shutdown
		if(forceShutdownIfNotRunning == true){
			// Pi not running - has it been on?
			if(pi_running == true){
				// Pi has been running and now isn't
				// so cut the power
				SleepyPiClass::enablePiPower(false);
				pi_running = false;
			}
		}
		return false;
	}

}
/* **************************************************+

	--checkPiStatus (current monitor)

	Monitor the current draw of the Rpi and deteremine
	whether the Raspberry Pi is running or not.

	The theshold_mA is the threshold below which the
	Rpi is considered shutdown in the traditional sense.
	For example a Rpi3 will settle on about 75mA after
	a "sudo shutdown -h now" command. As a rule of thumb
	anything nelow 90mA should be considered shutdown.

	Note a Pi Zero or A+ can have "active" currents as 
	low as 100mA

	Option to Force a shutdown i.e. remove power to
	the Raspberry Pi if it is not runnng.

+****************************************************/

bool SleepyPiClass::checkPiStatus(long threshold_mA, bool forceShutdownIfNotRunning)
{

	float 	pi_current = 0.0;

	// Read the RPi Current draw
	pi_current = SleepyPiClass::rpiCurrent();
	if(pi_current >= (float)threshold_mA)
	{
		// RasPi is still running
		pi_running = true;
		return true;
	}
	else
	{
		// Pi not running - either booting or manually shutdown
		if(forceShutdownIfNotRunning == true){
			// so cut the power
			SleepyPiClass::enablePiPower(false);
			pi_running = false;

		}
		return false;
	}

}
/* **************************************************+

	--piShutdown

	Set the Handshake lines to command a shutdown of
	the Raspberry Pi. Then wait for the Raspberry Pi
	to shutdown by monitoring the handshake line that 
	indicates that the Raspberry Pi is running. Once
	the RPi has shutdown remove the power after a 
	small guard interval.

	Option to Force a shutdown i.e. remove power to
	the Raspberry Pi if it is not handshaking.

+****************************************************/
void SleepyPiClass::piShutdown(void)
{
	int	handShake;
	unsigned long timeStart, timeNow, testTime;
	
	// Command the Sleepy Pi to shutdown
	if(simulationMode == true){
		// Serial.println("piShutdown()");
		// Switch off the Pi
		delay(5000);
		SleepyPiClass::enablePiPower(false);
	}
	else {

		digitalWrite(CMD_PI_TO_SHDWN_PIN,HIGH);		
		
		// Wait for the Pi to shutdown 
		timeStart = millis();
		testTime = 0;
		handShake = digitalRead(PI_IS_RUNNING);
		while((handShake > 0) && (testTime < kFAILSAFETIME_MS)){
			handShake = digitalRead(PI_IS_RUNNING);  
			delay(50);
			timeNow = millis();
			testTime = timeNow - timeStart;
		}
		// Switch off the Pi
		delay(5000); // delay to make sure the Pi has finished shutting down
		SleepyPiClass::enablePiPower(false);
		digitalWrite(CMD_PI_TO_SHDWN_PIN,LOW);	
	}

	return;
}
/* **************************************************+

	--piShutdown (current monitor)

	Set the Handshake lines to command a shutdown of
	the Raspberry Pi. Then wait for the Raspberry Pi
	to shutdown by monitoring the handshake line that 
	indicates that the Raspberry Pi is running. Once
	the RPi has shutdown remove the power after a 
	small guard interval.

	Option to Force a shutdown i.e. remove power to
	the Raspberry Pi if it is not handshaking.

+****************************************************/
void SleepyPiClass::piShutdown(long threshold_mA)
{
	// int	handShake;
	bool pi_running;
	unsigned long timeStart, timeNow, testTime;
	
	// Command the Sleepy Pi to shutdown
	if(simulationMode == true){
		// Serial.println("piShutdown()");
		// Switch off the Pi
		delay(5000);
		SleepyPiClass::enablePiPower(false);
	}
	else {

		digitalWrite(CMD_PI_TO_SHDWN_PIN,HIGH);		
		
		// Wait for the Pi to shutdown 
		timeStart = millis();
		testTime = 0;
		pi_running = SleepyPiClass::checkPiStatus(threshold_mA,false);
		while((pi_running == true) && (testTime < kFAILSAFETIME_MS)){
			pi_running = SleepyPiClass::checkPiStatus(threshold_mA,false); 
			delay(50);
			timeNow = millis();
			testTime = timeNow - timeStart;
		}

		// Switch off the Pi
		delay(5000); // delay to make sure the Pi has finished shutting down
		SleepyPiClass::enablePiPower(false);
		digitalWrite(CMD_PI_TO_SHDWN_PIN,LOW);	
	}

	return;
}

/* **************************************************+

	--rtcInit

	Setup the PCF8523 RTC registers with some essential 
	settings for Sleepy Pi functioning.


+****************************************************/
bool SleepyPiClass::rtcInit(bool reset)
{

	uint8_t tmp_reg;

	begin();
	// Check whether the RTC is detected - will read 0xFF if not
	tmp_reg = rtcReadReg(PCF8523_CONTROL_3);
	if(tmp_reg == 0xFF){
		//Not detected
		return false;
	}

	if(reset){
		SleepyPiClass::rtcReset();
	}
	// ...Disable default 32 kHz O/P on Alarm
	SleepyPiClass::rtcStop_32768_Clkout();   
	SleepyPiClass::setBatterySwitchover();
	SleepyPiClass::clearRtcInterruptFlags();
	SleepyPiClass::rtcCapSelect(eCAP_12_5pF);

	return true;
}
/* **************************************************+

	--rtcReset

	Reset the PCF8523 RTC 

+****************************************************/
void SleepyPiClass::rtcReset()
{	
	// Rest the RTC
	reset();
}
/* **************************************************+

	--rtcStop_32768_Clkout

	Stop the default 32khz clock being output on the
	/Alarm pin  

+****************************************************/
void SleepyPiClass::rtcStop_32768_Clkout()
{	
	// Stop the default 32kHz output on the INT1 pin as we use this
	// as an alarm
	stop_32768_clkout();
}

/* **************************************************+

	--rtcClearInterrupts

	Clear any active interrupt flags  

+****************************************************/
uint8_t SleepyPiClass::rtcClearInterrupts()
{	
	return clearRtcInterruptFlags();
}
/* **************************************************+

	--rtcIsRunning

	Find out if the RTC is currently running 
	or stopped 

+****************************************************/
uint8_t SleepyPiClass::rtcIsRunning()
{
	return isrunning();
}
/* **************************************************+

	--supplyVoltage

	Measure the Supply voltage of the External supply
	to the Sleepy Pi and return it as a scaled voltage
	in Volts.

	Note: This is not calibrated and should only be used 
	as a rough guide.

+****************************************************/
float SleepyPiClass::supplyVoltage(void)
{

	int 	reading;
	float   voltage;

	// Read
	reading = analogRead(V_SUPPLY_PIN);   
  
	// Convert
	voltage =  3.22 * (float)reading;     // Raw voltage reading
	// 10-bit ADC resolution = 3.3 / 1024 = 3.22mV
  
	return  voltage / 52;             	  // Scaled to Volts

}
/* **************************************************+

	--rpiCurrent

	Measure the Rpi current and return it as a
	scaled current in mA.

	Note: This is not calibrated and should only be used 
	as a rough guide. 

+****************************************************/
float SleepyPiClass::rpiCurrent(void)
{
	int 	reading;
	float   current;

	// Read
	reading = analogRead(I_MONITOR_PIN);  
	// remove lower bit noise
	if(reading <= 3)
	{
		reading = 0;
	}
	// Convert
	current =  3.22 * (float)reading;     // Raw current reading
	// 10-bit ADC resolution = 3.3 / 1024 = 3.22mV
  
	return  current; 					  // in mA            

}	

SleepyPiClass SleepyPi;
