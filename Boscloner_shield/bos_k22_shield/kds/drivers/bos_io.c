/*
 * bos_io.c
 *
 *  Created on: Feb 12, 2016
 *      Author: MJ
 */
#include "bos_io.h"

#define PB_DEBOUNCE_DEL	50


//gpio for boscloner board - init
void init_bosio() {

	SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;

	//PB1
	GPIOC_PDDR |= 0 << PB1_LOC;  //input=0 output=1
	PORTC_PCR7 |= PORT_PCR_MUX(1) ; //mux 1 = gpio, drive strength on
	//LED0
	GPIOC_PDDR |= 1 << LED0_LOC;  //input=0 output=1
	PORTC_PCR8 |= PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK; //mux 1 = gpio, drive strength on
	//LED0
	GPIOC_PDDR |= 1 << LED1_LOC;  //input=0 output=1
	PORTC_PCR9 |= PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK; //mux 1 = gpio, drive strength on

}


//debounce for pushbutton,  return1 if switch pushed.
uint8_t pb1_check() {
	if(!PB1_READ) {
		DelayMS(PB_DEBOUNCE_DEL);
		if(!PB1_READ) {
			while(!PB1_READ);	//wait until released.
			return 1;
		}
	}
	return 0;
}
