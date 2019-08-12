#include "wkup.h"
#include "delay.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_exti.h"

//1. In run mode, when PA0 occurs a rising edge, the interrupt routine of PA0 
//   will execute and check if pressing time is longer than 2s, then turn into standby mode.
//2. In standby mode, when PA0 occurs a rising edge, the program start from beginning. 
//   At the end of initialization, check if pressing time is longer than 2s, if not, back to standby mode,
//   otherwise, continue to run.

static uint8_t DEVICE_ON = 0;

//private functions
void Sys_Standby(void);
void Sys_Enter_Standby(void);
void EXTI0_IRQHandler(void);
uint8_t Check_WKUP(void);


//Green LED indicates wakeup status. It holds green during Check_WKUP status. It blinks when application is running.
static uint8_t LED_Green_Status = 0;
void Init_LED_Green() {
	GPIO_InitTypeDef GPIO_InitStructure;
	// Set GPIO for LED 13
	// Enable clock for GPIOC
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	// Configure PA13 as push-pull output
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void Turn_On_LED_Green() {
	// Reset bit will turn on LED (because the logic is interved)
	GPIO_ResetBits(GPIOC, GPIO_Pin_13);
	LED_Green_Status = 1;
}

void Turn_Off_LED_Green() {
	// Set bit will turn off LED (because the logic is interved)
	GPIO_SetBits(GPIOC, GPIO_Pin_13);
	LED_Green_Status = 0;
}


void Toggle_LED_Green() {
	if(LED_Green_Status) {
		Turn_Off_LED_Green();
	}
	else {
		Turn_On_LED_Green();
	}
}


//enter standby mode
void Sys_Standby(void)
{  
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);	//enable PWR clock
	PWR_WakeUpPinCmd(ENABLE);  //enable PA0 wakeup pin
	PWR_EnterSTANDBYMode();	  //enter standby mode 
}


//register standby functions for peripherals
static void (*Standby_Funcs[STANDBY_FUNCS_NUM])(void);
static uint8_t Standby_Funcs_Cnt = 0;

void Register_Standby_Funcs(void (*standby_func)(void)) {
	if(Standby_Funcs_Cnt < STANDBY_FUNCS_NUM - 1) {
		Standby_Funcs[Standby_Funcs_Cnt] = standby_func;
		Standby_Funcs_Cnt++;
	}
	DelayMs(10);
}

void Sys_Enter_Standby(void)
{			 
	uint8_t i;
	Turn_Off_LED_Green();
	//run peripherals' standby functions
	for(i = 0; i < Standby_Funcs_Cnt; i++) {
		Standby_Funcs[i]();
	}
	DelayMs(10);
	//disable clock;
	RCC_APB2PeriphResetCmd(0x01fc,DISABLE);//reset
	Sys_Standby();
}


//check if it is a real wakeup or noise	
uint8_t Check_WKUP(void) 
{
	uint8_t t=0;	//record time of pressing
	Turn_On_LED_Green(); // turn on LED green as indicator
	while(1)
	{ 
		if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0))
		{
			t++;			
			DelayMs(20);
			if(t>=100)		//press time is enough
			{
				//LED0=0;	 	
				return 1; 	
			}
		}else 
		{ 
			//LED0=1;
			return 0; 
		}
	}
}  


//PA0 interrupt
void EXTI0_IRQHandler(void)
{ 		    		    				     		    
	if(DEVICE_ON) {
		if(Check_WKUP()) { //check if it is a real turn off
			DEVICE_ON = 0;
			Sys_Enter_Standby();
		}
	}
	EXTI_ClearITPendingBit(EXTI_Line0); //clear interrupt
} 

//initialize wakeup module
void WKUP_Init(void)
{	
    GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
 
	//initialize gpio
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_0;	 
	GPIO_InitStructure.GPIO_Mode =GPIO_Mode_IPD;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);	
 
	//initialize external interrupt
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;			
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //trigger at rising edge
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);	
 
	// Set interrupt priority group
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	// Set System Timer IRQ at higher priority
	NVIC_SetPriority(SysTick_IRQn, 0);
	// Set PA0 priority
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure);
 
	if(Check_WKUP()==0) Sys_Standby(); //if not press 2s, still standby
	DEVICE_ON = 1;
		
}
