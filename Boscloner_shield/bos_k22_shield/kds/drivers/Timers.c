/******************************************************************
 * timers.c

 example:
 //setup the tick period and timer prescale values for timing

 main{
 InitTimer();
 t1 = RegisterTimer();
 TimerStart(t1, 1000L);   //starts t1, 1 second interval
 if(!TimerIsDone(t1)) {
 //Do something
  	  TimerStart(t1, 5000);	//restart the timer
 }//if
 }
 *******************************************************************/
//#include "System.h"
#include "Timers.h"
#include "stdint.h"
//#include "MKL25Z4.h"
#include "MK22F51212.h"

static uint16_t LED_BlinkSpeed = 0;
static uint16_t LED_BlinkTick = 0;

#define MAX_TIMERS  10
//global variables
static volatile int ntimers = 0; 	//counter for the n of counters registered
static volatile uint32_t tick = 0;  // tick counter
//static volatile uint32_t timers[MAX_TIMERS] = { 0UL };    // Timer array
static volatile uint16_t i;
volatile uint16_t t1;



typedef struct timers_ {
	uint8_t on ;
	uint32_t ticks;
}timers_t;

volatile static timers_t timers[MAX_TIMERS];


/***********************************************
 * ISR for the timers
 ************************************************/
//void __ISR(_CORE_TIMER_VECTOR, ipl2) CoreTimerHandler(void) {
void SysTick_Handler(void) {

	__asm("NOP");

	++tick;  //increment the tick value
	for (i = 0; i < ntimers; ++i) {
		//if (timers[i] > 0L) {
		//	--timers[i];
		if ( (timers[i].ticks) > 0L && timers[i].on>0 ) {
			--(timers[i].ticks);
		}
	}

//add any discrete system period tasks here
#ifdef USE_HEARTBEAT
	if (LED_BlinkSpeed == 0) {
		LED = 0;
		LED_BlinkTick = 0;
	} else if (LED_BlinkSpeed == 0xFFFF) {
		LED = 1;
		LED_BlinkTick = 0;
	} else {
		if (++LED_BlinkTick >= LED_BlinkSpeed) {
			LED_BlinkTick = 0;
			LED ^= 1;
			//TransmitString("testing output\r\n");
		}
	}
#endif
}	//isr


/*****************************************************************
 * init timer1
 ******************************************************************/
void InitTimers(void) {
	/*
	 ntimers      = 0;
	 // Initialize Timer1 for general timing
	 T1CON         = 0;           //Internal clock, 1:1 prescaler
	 TMR1          = 0;           // Clear timer register
	 PR1           = TICK_PERIOD; // Load the period value
	 IPC0bits.T1IP = 1;           // Set Timer1 Interrupt Priority Level
	 IFS0bits.T1IF = 0;           // Clear Timer1 Interrupt Flag
	 IEC0bits.T1IE = 1;           // Enable Timer1 interrupt
	 T1CONbits.TON = 1;           // Start Timer
	 * */



}
/****************************************************************
 * getTick - returns the current tick value
 * 	this only give access to the current tick value of
 *	this c file
 *****************************************************************/
uint32_t GetTick(void) {
	return tick;
}

/*****************************************************************
 * delay
 * delay general purpose blocking delay
 *****************************************************************/
//delay for tick milliseconds 
void DelayMS(uint16_t tickDel) {
	uint32_t t;

	t = tick + tickDel;
	while (tick < t)
		;
}
/******************************************************************
 * registerTimer
 *	registers a timer in array that will begin incrementing
 *******************************************************************/
uint16_t RegisterTimer(void) {
	int ret = -1; //error by default

	if (ntimers < MAX_TIMERS)//check to make sure dont overflow max timer array
	{
		//timers[ntimers] = 0; //sets value to 0 in the next array value available
		timers[ntimers].ticks = 0; //sets value to 0 in the next array value available
		ret = ntimers; //returns the nth timer that it is initialized to
		ntimers++; 	//increment the # of timers value
	}
	return ret; //return -1 if too many timers, or nth timer that was initialized
}

/***********************************************
 * timerStart
 *	input: passes the index of timer that want to start
 *		number of ticks to initialize to
 *	return: none
 *
 ************************************************/
void TimerStart(uint16_t timer, uint32_t ticks) {
	if (timer >= 0 && timer < ntimers) //check to make sure not negative and smaller than max size
		//timers[timer] = ticks; //initialize the timer tick value to that timer
		timers[timer].on = 1;	//turned on flag
		timers[timer].ticks = ticks; //initialize the timer tick value to that timer
}

// clears the on and ticks
//must be restart to run again
void TimerStop(uint16_t timer) {
	if (timer >= 0 && timer < ntimers) //check to make sure not negative and smaller than max size
		//timers[timer] = ticks; //initialize the timer tick value to that timer
		timers[timer].on = 0;	//turned on flag
		timers[timer].ticks = 0; //initialize the timer tick value to that timer
}

/**************************************************************
 * timer
 *	input: number of timer
 input the number of timer to stop, set the value to 0
 ***************************************************************/
void TimerPause(uint16_t timer) {
	//if (timer >= 0 && timer < ntimers)
		//timers[timer] = 0;
	if (timer >= 0 && timer < ntimers){
		timers[timer].on = 0;
	}
}

/****************************************************************
 * timerIsOn
 input: number of timer
 returns: 1 if running, 0 if done
 *****************************************************************/
uint16_t TimerIsOn(uint16_t timer) {
	//if (timer >= 0 && timer < ntimers && timers[timer] > 0)
	if (timer >= 0 && timer < ntimers && timers[timer].on > 0)
		return 1;
	else
		return 0;
}

/****************************************************************
 * timerIsDone
 input: number of timer
 returns: 1 finished
 *****************************************************************/
uint16_t TimerIsDone(uint16_t timer) {
	//if (timer >= 0 && timer < ntimers && timers[timer] > 0)
	if (timer >= 0 && timer < ntimers && timers[timer].ticks ==0  && timers[timer].on==1)
		return 1;
	else
		return 0;
}

/****************************************************************
 TICK_PERIOD is set to give me a 1 millisecond period at 80 MHz.

 The timer functions are used as:
 *****************************************************************/

