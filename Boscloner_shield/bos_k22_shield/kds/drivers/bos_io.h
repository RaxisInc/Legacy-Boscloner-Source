/*
 * bos_io.h
 *
 *  Created on: Feb 12, 2016
 *      Author: MJ
 */

#ifndef DRIVERS_BOS_IO_H_
#define DRIVERS_BOS_IO_H_

#include "MK22F51212.h"
#include "boscloner_app.h"


#define	PB1_LOC 7
#define PB1_HIGH  		(GPIOC_PSOR|= 1<<PB1_LOC)
#define PB1_LOW 		(GPIOC_PCOR|= 1<<PB1_LOC)
#define PB1_TOGGLE 		(GPIOC_PTOR|= 1<<PB1_LOC)
#define PB1_READ 		(GPIOC_PDIR & (1<<PB1_LOC))

#define	LED0_LOC 8
#define LED0_HIGH  		(GPIOC_PSOR|= 1<<LED0_LOC)
#define LED0_LOW 		(GPIOC_PCOR|= 1<<LED0_LOC)
#define LED0_TOGGLE 	(GPIOC_PTOR|= 1<<LED0_LOC)


#define	LED1_LOC 9
#define LED1_HIGH  		(GPIOC_PSOR|= 1<<LED1_LOC)
#define LED1_LOW 		(GPIOC_PCOR|= 1<<LED1_LOC)
#define LED1_TOGGLE 	(GPIOC_PTOR|= 1<<LED1_LOC)


void init_bosio();
uint8_t pb1_check();

#endif /* DRIVERS_BOS_IO_H_ */
