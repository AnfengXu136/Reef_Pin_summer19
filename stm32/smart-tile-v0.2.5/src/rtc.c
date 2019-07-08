#include <stdio.h>
#include "delay.h"
#include "rtc.h"
#include "stm32f10x_rtc.h"


_calendar_obj calendar;    // time struct

u8 const table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; // moth correction

const u8 mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31}; // days for each month


static void RTC_NVIC_Config(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

u8 RTC_Init(void)
{
u8 temp=0;
// check if config for the first time
RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE); // enable PWR and BKP as clock
PWR_BackupAccessCmd(ENABLE); // enable backup register access
if (BKP_ReadBackupRegister(BKP_DR1) != RTC_DATA) // read data from backup register
// if not match
{
BKP_DeInit(); // reset backup area
RCC_LSEConfig(RCC_LSE_ON); //set LSE
while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET&&temp<250)
// check LSE state
{
temp++;
DelayMs(10);
}
if(temp>=250)return 1; // init fail
RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE); //set RTC
//(RTCCLK) select LSE as RTC source
RCC_RTCCLKCmd(ENABLE); // enable RTC
RTC_WaitForLastTask(); // wait for last write operation
//RTC_WaitForSynchro();
//RTC_ITConfig(RTC_IT_SEC, ENABLE); // enable interrupt
//RTC_WaitForLastTask(); //wait for last write operation
RTC_EnterConfigMode();
RTC_SetPrescaler(32767); //set RTC prescaler
RTC_WaitForLastTask(); //wait for last write operation
RTC_Set(2019,6,10,3,23,55); // set time
RTC_ExitConfigMode();
BKP_WriteBackupRegister(BKP_DR1, RTC_DATA); // write to backup register
// write user data RTC_DATA
}
else//continue for RTC
{
//RTC_WaitForSynchro();
//RTC_ITConfig(RTC_IT_SEC, ENABLE); // enable interrupt
//RTC_WaitForLastTask(); //wait for last write operation
}
//RTC_NVIC_Config(); //RTC interrupt config
RTC_Get(); // udpate time
return 0;
}


u8 Is_Leap_Year(u16 pyear)
{
    if(pyear%4==0) // divide by 4
    {
        if(pyear%100==0)
        {
            if(pyear%400==0)    return 1;
            else    return 0;
        }
        else
            return 1;
    }
    else
        return 0;
}

u8 RTC_Set(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec)
{
u16 t;
u32 seccount=0;
if(syear<1970||syear>2099)return 1;
for(t=1970;t<syear;t++) // add up all seconds for all years
{ if(Is_Leap_Year(t))seccount+=31622400;// seconds for leap year
else seccount+=31536000; // seconds for normal year
}
smon-=1;
for(t=0;t<smon;t++) // add up seconds for previous months
{ seccount+=(u32)mon_table[t]*86400; // add up seconds for months
if(Is_Leap_Year(syear)&&t==1)seccount+=86400;// add one day for leap year
}
seccount+=(u32)(sday-1)*86400; // add up seconds for previous days
seccount+=(u32)hour*3600; // hours
seccount+=(u32)min*60; // minutes
seccount+=sec; // seconds
RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE); // enable PWR and BKP clock
PWR_BackupAccessCmd(ENABLE); // enable backup register access
RTC_SetCounter(seccount); // set registers
RTC_WaitForLastTask(); // wait for finish
return 0;
}


u8 RTC_Get(void)
{ static u16 daycnt=0;
u32 timecount=0;
u32 temp=0;
u16 temp1=0;
timecount=RTC->CNTH; // get seconds
timecount<<=16;
timecount+=RTC->CNTL;
temp=timecount/86400; // get days
if(daycnt!=temp) // exceed one day
{
daycnt=temp;
temp1=1970; //from 1970
while(temp>=365)
{
if(Is_Leap_Year(temp1))
{
if(temp>=366)temp-=366; // seconds for leap year
else break;
}
else temp-=365; // normal year
temp1++;
}
calendar.w_year=temp1; //get year
temp1=0;
while(temp>=28) // exceed one month
{
if(Is_Leap_Year(calendar.w_year)&&temp1==1)// leap year and Feb
{
if(temp>=29)temp-=29;// seconds for leap year
else break;
}
else
{ if(temp>=mon_table[temp1])temp-=mon_table[temp1];// normal year
else break;
}
temp1++;
}
calendar.w_month=temp1+1; // get month
calendar.w_date=temp+1; // get date
}
temp=timecount%86400; // get seconds
calendar.hour=temp/3600; // get hours
calendar.min=(temp%3600)/60; // get minutes
calendar.sec=(temp%3600)%60; // get seconds
// calendar.week=RTC_Get_Week(calendar.w_year,calendar.w_month,calendar.w_date);
// get week
return 0;
}

void RTC_IRQHandler(void)
{
if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
{
RTC_Get();
}
if(RTC_GetITStatus(RTC_IT_ALR)!= RESET)
{
RTC_ClearITPendingBit(RTC_IT_ALR);
RTC_Get();
}
RTC_ClearITPendingBit(RTC_IT_SEC|RTC_IT_OW);
RTC_WaitForLastTask();
}
