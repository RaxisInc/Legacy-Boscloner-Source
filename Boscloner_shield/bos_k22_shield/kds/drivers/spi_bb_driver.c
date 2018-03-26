/*
 * spi_driver.c
 *
 *  Created on: Dec 15, 2015
 *      Author: MJ
 */
#include "MK22F51212.h"
#include "spi_bb_driver.h"

//#define BOARD_VERSION_FRDMK22
#define BOARD_VERSION_R0_2
//#define BOARD_VERSION_R0_3
//pins to use on the k22f freedom board:
//sck = ptd5
//miso = ptd7
//mosi = ptd6
//cs = ptd4
//c/d = pta1
//reset =ptb19

//Bosclone shield board
//sck = PTD1
//mosi = PTD2
//cs = t1 (r0.2)ptc0  = R0.3 CHANGE = P45=PTC2
//c/d = r0.2 = ptc6 = r0.3
//reset = ptc4 (r0.2) ; r0.3 = ptc4

void spi_bb_init(void) {

	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTC_MASK
									| SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK;


#ifdef	BOARD_VERSION_R0_2
	//c/D data pin = ptC6
	GPIOC_PDDR |= 1 << 6;  //input=0 output=1
	PORTC_PCR6 |= PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK; //mux 1 = gpio, drive strength on
	//reset data pin = ptC4
	GPIOC_PDDR |= 1 << 4;  //input=0 output=1
	PORTC_PCR4 |= PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK; //mux 1 = gpio, drive strength on
	//sck data pin = ptD1
	GPIOD_PDDR |= 1 << 1;  //input=0 output=1
	PORTD_PCR1 |= PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK; //mux 1 = gpio, drive strength on
	//mosi data pin = ptd2
	GPIOD_PDDR |= 1 << 2;  //input=0 output=1
	PORTD_PCR2 |= PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK; //mux 1 = gpio, drive strength on
	//cs data pin = ptC0
	GPIOC_PDDR |= 1 << 0;  //input=0 output=1
	PORTC_PCR0 |= PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK; //mux 1 = gpio, drive strength on
#endif


#ifdef BOARD_VERSION_FRDMK22
	//c/D data pin = pta1
	GPIOA_PDDR |= 1 << 1;  //input=0 output=1
	PORTA_PCR1 |= PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK; //mux 1 = gpio, drive strength on
	//reset data pin = ptB19
	GPIOB_PDDR |= 1 << 19;  //input=0 output=1
	PORTB_PCR19 |= PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK; //mux 1 = gpio, drive strength on
	//sck data pin = ptd5
	GPIOD_PDDR |= 1 << 5;  //input=0 output=1
	PORTD_PCR5 |= PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK; //mux 1 = gpio, drive strength on
	//mosi data pin = ptd6
	GPIOD_PDDR |= 1 << 6;  //input=0 output=1
	PORTD_PCR6 |= PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK; //mux 1 = gpio, drive strength on
	//miso data pin = ptd7
	GPIOD_PDDR |= 1 << 7;  //input=0 output=1
	PORTD_PCR7 |= PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK; //mux 1 = gpio, drive strength on
	//cs data pin = ptd4
	GPIOD_PDDR |= 1 << 4;  //input=0 output=1
	PORTD_PCR4 |= PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK; //mux 1 = gpio, drive strength on
#endif

}

void testSPIBB(uint8_t d) {

	for (int i = 0; i < 8; i++) {
		fastSPIwrite(i);
	}
}

void testIO(void) {


	PIN_C_D_TOGGLE;
	PIN_RST_TOGGLE;
	PIN_SCK_TOGGLE;
	PIN_MOSI_TOGGLE;
	PIN_CS_TOGGLE;

}

