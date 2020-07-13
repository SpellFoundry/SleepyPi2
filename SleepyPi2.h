/*
 * SleepyPi2.h - library for Sleepy Pi power management board
 */

#ifndef SLEEPYPI2_h
#define SLEEPYPI2_h

#include "Arduino.h"
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <PCF8523.h>
#include <TimeLib.h>
#include <Wire.h>
#include <LowPower.h>

//#define DEBUG_MESSAGES

#define kFAILSAFETIME_MS		30000		// Failsafe shutdown time in milliseconds
#define kONBUTTONTIME_MS		3000		// 
#define kFORCEOFFBUTTONTIME_MS	1000

// Typical shutdown currents of various models of RPis in mA
typedef enum {
	eZero 		= 48,   
	eA        	= 70,   // 
	eA_Plus		= 55,   
	eB 			= 75,	 
	eB_PLUS 	= 73,	
	eB2         = 70,
	eB3         = 75
} eRPI_SHUTDOWN_CURRENT;

// library interface description
//class SleepyPiClass {
class SleepyPiClass : public PCF8523 , public LowPowerClass {
  
	// user-accessible "public" interface
  public:
	bool	simulationMode;
	bool	power_on;
	bool	ext_power_on;
	bool	pi_running;

	// TODO Timer

    SleepyPiClass();		// Look at initialising the simulationMode from the constructor

	// Power On Off
	void enableExtPower(bool enable);
	void enablePiPower(bool enable);

	// Control
	void  startPiShutdown(void);
	bool  checkPiStatus(bool forceShutdownIfNotRunning);
	bool  checkPiStatus(long threshold_mA, bool forceShutdownIfNotRunning);
	void  piShutdown(void);
	void  piShutdown(long threshold_mA);

	// Time
	bool rtcInit(bool reset); 
	void rtcReset(void);
	void rtcStop_32768_Clkout(void);
	void rtcStartCounter1(uint8_t value);
	uint8_t rtcClearInterrupts(void);
	uint8_t rtcIsRunning(void);

	// Wakeup
	void enableWakeupAlarm(bool enable);

	// Monitoring
	float supplyVoltage(void);
	float rpiCurrent(void);	

  private:
    static bool exists;
	bool	simPiOn;

//	time_t wakeupTime;

};

extern SleepyPiClass SleepyPi;

#endif // SLEEPYPI2_h
 
