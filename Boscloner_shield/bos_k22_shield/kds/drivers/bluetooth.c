/*
 * bluetooth.c
 *
 *  Created on: Feb 1, 2016
 *      Author: MJ
 */
#include "bluetooth.h"

void init_bt(){
	//bluetooth enable =pta4
	//state = pta2

		SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;
		//enable = pta5
		GPIOA_PDDR |= 1 << 5;  //input=0 output=1
		PORTA_PCR5 |= PORT_PCR_MUX(0X01) | PORT_PCR_DSE_MASK; //
		BT_EN();

		//state = pta2
		GPIOA_PDDR |= 0 << 2;  //input=0 output=1
		PORTA_PCR2 = 0;		//for some reason had to write a 0 before it would accept the new values...
		PORTA_PCR2 |= PORT_PCR_MUX(0X01) | PORT_PCR_DSE_MASK; //


}


#define BT_DEL 0
void bt_test_messages() {

	uart_putString("$!SCAN,a9:78:65:43:21?$\r");
	DelayMS(BT_DEL);
	uart_putString("$!CLONE,a9:78:65:43:22?$\r");
	DelayMS(BT_DEL);
	uart_putString("$!STATUS,MCU STARTUP?$\r");
	DelayMS(BT_DEL);
	uart_putString("$!SCAN,a9:78:65:43:23?$\r");
	DelayMS(BT_DEL);
	uart_putString("$!CLONE,a9:78:65:43:24?$\r");
	DelayMS(BT_DEL);
	uart_putString("$!STATUS,Done!?$\r");
	DelayMS(2000);
}

