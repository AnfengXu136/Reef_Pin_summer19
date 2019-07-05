/******************************************************************************
ms5803.h
Library for ms5803 pressure sensors.
Casey Kuhns @ SparkFun Electronics
6/26/2014
https://github.com/sparkfun/ms5803-14BA_Breakout

The MS58XX MS57XX and MS56XX by Measurement Specialties is a low cost I2C pressure
sensor.  This sensor can be used in weather stations and for altitude
estimations. It can also be used underwater for water depth measurements. 

In this file are the function prototypes in the ms5803 class 

Resources:
This library uses the Arduino Wire.h to complete I2C transactions.

Development environment specifics:
	IDE: Arduino 1.0.5
	Hardware Platform: Arduino Pro 3.3V/8MHz
	ms5803 Breakout Version: 1.0

**Updated for Arduino 1.6.4 5/2015**

This code is beerware. If you see me (or any other SparkFun employee) at the
local pub, and you've found our code helpful, please buy us a round!

Distributed as-is; no warranty is given.
******************************************************************************/

#ifndef __ms5803_H
#define __ms5803_H

#include "stm32f10x.h"


// Define units for conversions. 
enum temperature_units
{
	CELSIUS,
	FAHRENHEIT,
};

// Define measurement type.
enum measurement
{	
	PRESSURE = 0x00,
	TEMPERATURE = 0x10
};

// Define constants for Conversion precision
enum precision
{
	ADC_256  = 0x00,
	ADC_512  = 0x02,
	ADC_1024 = 0x04,
	ADC_2048 = 0x06,
	ADC_4096 = 0x08
};

// Define address choices for the device (I2C mode)
enum ms5803_addr
{
	ADDRESS_HIGH = 0x76,
	ADDRESS_LOW  = 0x77
};

//Commands
#define CMD_RESET 0x1E // reset command 
#define CMD_ADC_READ 0x00 // ADC read command 
#define CMD_ADC_CONV 0x40 // ADC conversion command 

#define CMD_PROM 0xA0 // Coefficient location


// class ms5803

void ms5803_Init(enum ms5803_addr address); 
void ms5803_Reset(void);	 //Reset device
uint8_t ms5803_Begin(void); // Collect coefficients from sensor

// Return calculated temperature from sensor
float ms5803_GetTemperature(enum temperature_units units, enum precision _precision);
// Return calculated pressure from sensor
float ms5803_GetPressure(enum precision _precision);
void ms5803_GetMeasurements(enum precision _precision);
void ms5803_demo(void);
void ms5803_getDepthAndPressure(float * p_depth, float * p_pressure);

#endif
