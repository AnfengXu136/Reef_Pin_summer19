/******************************************************************************
ms5803.cpp
Library for ms5803 pressure sensor.
Casey Kuhns @ SparkFun Electronics
6/26/2014
https://github.com/sparkfun/ms5803-14BA_Breakout

The MS58XX MS57XX and MS56XX by Measurement Specialties is a low cost I2C pressure
sensor.  This sensor can be used in weather stations and for altitude
estimations. It can also be used underwater for water depth measurements. 

In this file are the functions in the ms5803 class

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

//#include <Wire.h> // Wire library is used for I2C
#include "delay.h"
#include "i2c.h"
#include "usart.h"

#include "ms5803.h"

#include <math.h>

enum ms5803_addr _address; 		// Variable used to store I2C device address.
uint16_t coefficient[8];// Coefficients;

int32_t _temperature_actual;
int32_t _pressure_actual;

void sendCommand(uint8_t command);	// General I2C send command function
uint32_t getADCconversion(enum measurement _measurement, enum precision _precision);	// Retrieve ADC result
void sensorWait(uint32_t time); // General delay function see: delay()


//------------------------------------------------
void ms5803_Init(enum ms5803_addr address) {
	i2c_init();
	_address = address;
	ms5803_Reset();
	ms5803_Begin();
}

void ms5803_Reset(void)// Reset device I2C
{
   sendCommand(CMD_RESET);
   sensorWait(3);
}

uint8_t ms5803_Begin(void)
// Initialize library for subsequent pressure measurements
{  
	uint8_t i;
	uint8_t buf[10];
	for(i = 0; i <= 7; i++)
  {
		uint8_t highByte; 
		uint8_t lowByte;
		sendCommand(CMD_PROM + (i * 2));
		i2c_read_multi_no_reg(_address, 2, buf);
		highByte = buf[0];
		lowByte = buf[1];
		coefficient[i] = (highByte << 8)|lowByte;
 	  // Uncomment below for debugging output.
		// USART2_PutChar('C');
		// USART2_PutInt(i);
		// USART2_PutString("= ");
		// USART2_PutInt(i);
		// USART2_PutInt(coefficient[i]);
		// USART2_PutString("\n");
	}

	return 0;
}
	
float ms5803_GetTemperature(enum temperature_units units, enum precision _precision)
// Return a temperature reading in either F or C.
{
	float temperature_reported;
	ms5803_GetMeasurements(_precision);
	// If Fahrenheit is selected return the temperature converted to F
	if(units == FAHRENHEIT)
  {
		temperature_reported = _temperature_actual / 100.0f;
		temperature_reported = (((temperature_reported) * 9) / 5) + 32;
		return temperature_reported;
	}
		
	// If Celsius is selected return the temperature converted to C	
	else
  {
		temperature_reported = _temperature_actual / 100.0f;
		return temperature_reported;
	}
}

float ms5803_GetPressure(enum precision _precision)
// Return a pressure reading units Pa.
{
	float pressure_reported;
	ms5803_GetMeasurements(_precision);
	pressure_reported = _pressure_actual;
	pressure_reported = pressure_reported / 10.0f;
	return pressure_reported;
}

void ms5803_GetMeasurements(enum precision _precision)

{
	//Retrieve ADC result
	int32_t temperature_raw = getADCconversion(TEMPERATURE, _precision);
	int32_t pressure_raw = getADCconversion(PRESSURE, _precision);
	
	
	//Create Variables for calculations
	int32_t temp_calc;
	int32_t pressure_calc;
	
	int32_t dT;
	
	//Now we have our first order Temperature, let's calculate the second order.
	int64_t T2, OFF2, SENS2, OFF, SENS; //working variables
		
	//Now that we have a raw temperature, let's compute our actual.
	dT = temperature_raw - ((int32_t)coefficient[5] << 8);
	temp_calc = (((int64_t)dT * coefficient[6]) >> 23) + 2000;
	
	// TODO TESTING  _temperature_actual = temp_calc;
	
	//Now we have our first order Temperature, let's calculate the second order.

	if (temp_calc < 2000) 
	// If temp_calc is below 20.0C
	{	
		T2 = 3 * (((int64_t)dT * dT) >> 33);
		OFF2 = 3 * ((temp_calc - 2000) * (temp_calc - 2000)) / 2;
		SENS2 = 5 * ((temp_calc - 2000) * (temp_calc - 2000)) / 8;
		
		if(temp_calc < -1500)
		// If temp_calc is below -15.0C 
		{
			OFF2 = OFF2 + 7 * ((temp_calc + 1500) * (temp_calc + 1500));
			SENS2 = SENS2 + 4 * ((temp_calc + 1500) * (temp_calc + 1500));
		}
    } 
	else
	// If temp_calc is above 20.0C
	{ 
		T2 = 7 * ((uint64_t)dT * dT)/pow(2,37);
		OFF2 = ((temp_calc - 2000) * (temp_calc - 2000)) / 16;
		SENS2 = 0;
	}
	
	// Now bring it all together to apply offsets 
	
	OFF = ((int64_t)coefficient[2] << 16) + (((coefficient[4] * (int64_t)dT)) >> 7);
	SENS = ((int64_t)coefficient[1] << 15) + (((coefficient[3] * (int64_t)dT)) >> 8);
	
	temp_calc = temp_calc - T2;
	OFF = OFF - OFF2;
	SENS = SENS - SENS2;

	// Now lets calculate the pressure
	

	pressure_calc = (((SENS * pressure_raw) / 2097152 ) - OFF) / 32768;
	
	_temperature_actual = temp_calc ;
	_pressure_actual = pressure_calc ; // 10;// pressure_calc;
	

}

uint32_t getADCconversion(enum measurement _measurement, enum precision _precision)
// Retrieve ADC measurement from the device.  
// Select measurement type and precision
{	
	uint32_t result;
	uint8_t highByte = 0, midByte = 0, lowByte = 0;
	uint8_t buf[3];
	
	sendCommand(CMD_ADC_CONV + _measurement + _precision);
	// Wait for conversion to complete
	sensorWait(1); //general delay
	switch( _precision )
	{ 
		case ADC_256 : sensorWait(1); break; 
		case ADC_512 : sensorWait(3); break; 
		case ADC_1024: sensorWait(4); break; 
		case ADC_2048: sensorWait(6); break; 
		case ADC_4096: sensorWait(10); break; 
	}	
	
	sendCommand(CMD_ADC_READ);
	i2c_read_multi_no_reg(_address, 3, buf);
	highByte = buf[0];
	midByte = buf[1];
	lowByte = buf[2];
	
	result = ((uint32_t)highByte << 16) + ((uint32_t)midByte << 8) + lowByte;

	return result;

}

//------------------------------------------------


void sendCommand(uint8_t command)
{	
	i2c_write_no_reg(_address, command);
	
}

void sensorWait(uint32_t time)
// Delay function.  This can be modified to work outside of Arduino based MCU's
{
	DelayMs(time);
}

//------------------------------------------------
void ms5803_demo() {
	float temp, pres;
	temp = ms5803_GetTemperature(CELSIUS, ADC_256);
	pres = ms5803_GetPressure(ADC_4096);
	printf("temp:\t%.3f\n", temp);
	printf("pressure:\t%.3f\n", pres);
}


float mbar_to_meter(float pressure) {
	// 1000 mbar = 1019.72 cm of water
	return pressure / 1000 * 10.1972;
}

static float init_pressure = 0;

void ms5803_getDepthAndPressure(float * p_depth, float * p_pressure) {
	float delta_pressure, depth, tmp_pressure;
	if (init_pressure == 0) init_pressure = ms5803_GetPressure(ADC_4096);
	tmp_pressure = ms5803_GetPressure(ADC_4096);
	delta_pressure = tmp_pressure - init_pressure;
	depth = mbar_to_meter(delta_pressure);
	*p_depth = depth;
	*p_pressure = tmp_pressure;
}
