/***************************************************************
 * timers.h
 ****************************************************************/
#include "stdint.h"

#ifndef _TIMERS_H
#define _TIMERS_H


//timers
extern volatile uint16_t t1;
//function prototypes

void SysTick_Handler(void);
void InitTimers(void);
uint32_t GetTick(void);
void DelayMS(uint16_t tickDel);
uint16_t RegisterTimer(void);
void TimerStart(uint16_t timer, uint32_t ticks);
void TimerStop(uint16_t timer);
uint16_t TimerIsOn(uint16_t timer);

#endif

