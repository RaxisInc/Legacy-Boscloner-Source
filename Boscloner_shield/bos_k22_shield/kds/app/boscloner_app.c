/*
 * boscloner.c
 *
 *  Created on: Jan 23, 2016
 *      Author: MJ
 */

#include "boscloner_app.h"

void Init_App();




int main(void) {

	Init_App();

	init_disp_update();
	printf("Startup......\n");
	printf("Boscloner Verion:%s\n", BOSCLONER_VERSION);
	printf("Clone Status: %u\n", clone_status);
	display();

	for (;;) // Forever loop
			{

		//BOSLONER PROCESSES
		if (TimerIsDone(dsp_tmr1)) { //check is display update time is done
			disp_revert_buf();
			display();
			TimerStop(dsp_tmr1);
		}

		if (pb1_check()) {	//don't update if update still being displayed
			clone_status ^= 1;	//toggle the clone status.
			print_clone_status(NULL);
		}

		if (WiegandDone() || cmd_done) { //or if cmd received from android app...(bluetooth).

			if (cmd_done) {	//rx uart command
				__asm("NOP");
				cmd_process();	//processed the rx uart cmd
				reset_cmd_state();
				uart_putString("$!STATUS,MCU ACK?$\r");	//MUST SEND \R IN ORDER FOR THE BT TO SEND THE DATA.
			}

			if (WiegandDone()) {	//decoded wiegand packet
				wiegand_process();	//process card data received from wiegand
				if (clone_status == 1) {
					//send_clone_cmd();//clone to pm3 only if autoclone is enabled
					clone_card();
					//send_app_status();
				} else {
					send_app_card_scan();
				}
				resetWeigand();		//reset weigand state machine
				printRecivedCardWiegand(NULL);	//print card data to lcd
			}

		} //rx weigand
	} //for

} //main

//send clone command to the pm3
void clone_card() {
	send_clone_cmd_pm3();	//send clone packet to the pm3
	send_app_card_clone();	//!this causes teh uart to stop working somehow....
}

void test_send_packets() {
//test_send_packet();	//WORKS!
//test_send2_packet();	//does not work....!
//test_send_packet3();	//works
}

void Init_App() {
	uint32_t val1;

	init_bosio();
	LED1_HIGH;
	LED0_HIGH;

	init_printf(NULL, putCharLCD);	//add the lcd write() as the callback
	LCD_Init();
	display();	//enable display
	bosclone_display();

	InitWeigand();
	init_UART();
	init_bt();
	spi_bb_init();	//init spi pins
	init_dspi();

	//CHECK THE CLOCK VALUES
	val1 = CLOCK_SYS_GetSystemClockFreq();

	//SETUP THE LCD
	LCD_Init();
	bosclone_display();
	display();	//enable display

	InitSysTimers();
	InitWeigandPins();

	//REGISTER TIMER FOR DISPLAY UPDATES
	dsp_tmr1 = RegisterTimer();
	LED1_LOW;
	LED0_LOW;

}

uint8_t update_being_displayed() {
	return (TimerIsOn(dsp_tmr1));

}




////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
