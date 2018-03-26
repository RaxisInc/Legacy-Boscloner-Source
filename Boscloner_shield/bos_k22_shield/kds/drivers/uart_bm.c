/*
 /*
 * uart_hal.c
 *
 *  Created on: Dec 21, 2015
 *      Author: MJ
 */

#include "MK22F51212.h"
#include "boscloner_app.h"
#include "uart_bm.h"
#include "string.h"
#include "printf.h"
#include "weigand.h"


void clear_data();
void cmd_process();
uint16_t atohex(uint8_t c) ;
void send_app_card_scan() ;
void send_app_status() ;
void send_app_card_clone() ;
void test_print_card_uart() ;
void ascii_array_tohex(uint8_t * pArray) ;
uint32_t get_card_hi_app() ;
uint32_t get_card_lo_app() ;
uint8_t charToHexDigit_test(uint8_t c) ;
unsigned char stringToByte(char c[2]) ;
void delay(int del) ;
void reset_cmd_state() ;
void clear_hex_vals() ;
void clear_cmd_val() ;
void clear_data() ;
void init_UART() ;
void putc_uart_printf(void* p, char c) ;



//UART PINS
//#define MK_UART_TXO PTE0
//MK_uart_RXI PTE1
#define MK_UART_TXO_LOC 	0
#define MK_UART_RXI_LOC 	0

//#define UART_LOOPBACK

#define FRDM_BRD
#define UART_INTERRUPT
//FREEDOM BOARD
//RXI = PTE1  ; TXO = PTE0

#define BAUD_RATE 9600

static volatile uint8_t c, flag;

#define APP_CARD_CLONE_STRING "APP CLONE"
#define APP_CLONE_STATUS_STRING "APP Change Status"

//void UART1_IRQHandler(void) {
#define CMD_CLONE		"CLONE"
#define CMD_CLONE_DIS	"DISABLE_CLONE"	//sent from android app
#define CMD_CLONE_EN	"ENABLE_CLONE"	//sent from android app
#define CMD_SCAN		"SCAN"
#define CMD_SYNC		"SYNC"
#define CMD_START		'$'
#define CMD_SEPERATOR 	':'
#define CMD_DATA_SIZE	32
#define CMD_VAL_SIZE 	16

//convert  the ascii arry to a hex value
#define CARD_ID_VALS_SIZE 10
static uint64_t card_hex = 0;
static uint8_t hexvals[CARD_ID_VALS_SIZE] = { 0 };
static uint32_t hi2, hi, lo;

#define CMD_STATE_WAIT 			0
#define CMD_STATE_START1	 	1
#define CMD_STATE_VAL 			2
#define CMD_STATE_DATA 			3
volatile uint8_t cmd_state = CMD_STATE_WAIT;

volatile uint8_t cmd_done = 0;
volatile uint8_t cmd_data[CMD_DATA_SIZE] = { 0 };
static volatile uint8_t cmd_val[CMD_VAL_SIZE] = { 0 };
static volatile uint8_t cmd_data_over = 0;
static volatile uint8_t cmd_val_over = 0;
volatile uint8_t clone_status = 1; //global flag to enable disable auto cloning auto enable
uint8_t charToHexDigit_test(uint8_t c);
uint32_t hex_to_ascii(uint8_t hex);

//$!CLONE,45:67:87:89:00?$	//receive from android
//$!SCAN,a9:78:65:43:21?$	//send to android
volatile uint8_t i = 0, j = 0;
void UART1_RX_TX_IRQHandler(void) {

	if (UART1_S1 & UART_S1_RDRF_MASK) {
		c = UART1_D;
		flag = 1;
#ifdef UART_LOOPBACK
		UART1_D = c;
#endif
	}

	//decode state machine
	switch (cmd_state) {
	case (CMD_STATE_WAIT):
		if (c == '$' && cmd_done == 0) { //don't start unles the cmd_done is processed (cleared).
			cmd_state = CMD_STATE_START1;
		}
		break;
	case (CMD_STATE_START1):
		if (c == '!') {
			cmd_state = CMD_STATE_VAL;
		} else {
			cmd_state = CMD_STATE_WAIT;
		}

		break;
		//GET THE COMMAND
	case (CMD_STATE_VAL):
		if (i > CMD_VAL_SIZE) {
			cmd_state = CMD_STATE_WAIT;	//reset state machine
			cmd_val_over = 1;
			i = 0;
		}

		if (c == ',') {	//finished receiving cmd value
			cmd_state = CMD_STATE_DATA;
		} else if (c == '?' || c == 0) {	//no data in packet, finished
			cmd_state = CMD_STATE_WAIT;   //data done
			cmd_done = 1;
			UART1_C2 &= ~UART_C2_RE_MASK & ~UART_C2_TE_MASK; //turn off tx/rx until processed.
		} else {
			cmd_val[i++] = c;
		}

		break;
		//GET THE DATA
	case (CMD_STATE_DATA):
		if (j > CMD_DATA_SIZE) {
			cmd_state = CMD_STATE_WAIT;	//reset state machine
			cmd_data_over = 1;
			cmd_done = 1;
			UART1_C2 &= ~UART_C2_RE_MASK & ~UART_C2_TE_MASK;
			i = 0;
			break;
		}
		if (c == ':') {	//finished receiving cmd value
			{
				__asm("NOP");
			}
			//j++;	//dont do anything
		} else if (c == '?' || c == ',') {
			cmd_state = CMD_STATE_WAIT;   //data done
			cmd_done = 1;
			UART1_C2 &= ~UART_C2_RE_MASK & ~UART_C2_TE_MASK;
			break;
		} else {
			cmd_data[j++] = c;
		}

		break;
	default:
		cmd_state = CMD_STATE_WAIT;
		break;

	}

	__asm("NOP");
}

//issue:  if tyring to print a command for within this funciton it will stall for some reason.
void cmd_process() {
	uint8_t temp;
	uint8_t junk[] = "JUNK";

	if (!strcmp(CMD_CLONE, cmd_val)) {
		//clone_status = 0;
		ascii_array_tohex(cmd_data);
		send_clone_cmd_pm3();	//send the clone command
		//uart_putString("$!STATUS,CLONE DONE!?$\r"); //this doesn' work now!
		printRecivedCardApp(APP_CARD_CLONE_STRING);
		__asm("NOP");
	}

	else if (!strcmp(CMD_CLONE_DIS, cmd_val)) {
		clone_status = 0;
		print_clone_status(APP_CLONE_STATUS_STRING);
	}

	else if (!strcmp(CMD_CLONE_EN, cmd_val)) {
		clone_status = 1;
		print_clone_status(APP_CLONE_STATUS_STRING);
	}

	else if (!strcmp(CMD_SYNC, cmd_val)) {
		clone_status = 1;
		print_clone_status(APP_CLONE_STATUS_STRING);
	}
}

uint16_t atohex(uint8_t c) {
	//hex[0] = hexDigit(c / 0x10);
	//hex[1] = hexDigit(c % 0x10);
	//hex[2] = '\0';
	return (c / 0x10);
}

void send_app_card_scan() {
	uint8_t * pid5;
	uint8_t temp, a, i;
	//example  uart_putString("$!SCAN,a9:78:65:43:21?$\r");
	pid5 = get_card_id5();
	init_printf(NULL, putc_uart_printf);	//setup to print to uartputc
	uart_putString("$!SCAN,");	//start command
	for (i = 0; i < 5; i++) {
		temp = *pid5++;
		//print nibble at a time
		printf("%1x", temp >> 4);
		printf("%1x", temp & 0xf);
		if (i != 4) {
			printf(":");
		}
	}
	uart_putString("?$\r");  //end command

	//re-enable print for lcd
	init_printf(NULL, putCharLCD);

}

void send_app_status() {
	uart_putString("$!STATUS,Clone Done!\r");
}

void send_app_card_clone() {
	uint8_t * pid5;
	uint8_t temp, a, i;
	//example  uart_putString("$!SCAN,a9:78:65:43:21?$\r");
	pid5 = get_card_id5();
	init_printf(NULL, putc_uart_printf);	//setup to print to uartputc
	uart_putString("$!CLONE,");	//start command
	for (i = 0; i < 5; i++) {
		temp = *pid5++;
		//print nibble at a time
		printf("%1x", temp >> 4);
		printf("%1x", temp & 0xf);
		if (i != 4) {
			printf(":");
		}
	}
	uart_putString("?$\r");  //end command

	//re-enable print for lcd
	init_printf(NULL, putCharLCD);

}

const uint8_t test_id_array[5] = { 0x9f, 0x30, 0xe2, 0x06, 0x20 };
void test_print_card_uart() {
	uint8_t * pid5;
	uint8_t temp, a, i;
	//example  uart_putString("$!SCAN,a9:78:65:43:21?$\r");
	init_printf(NULL, putc_uart_printf);	//setup to print to uartputc
	uart_putString("$!SCAN,");	//start command
	for (i = 0; i < 5; i++) {
		temp = test_id_array[i];
		//print nibble at a time
		printf("%1x", temp >> 4);
		printf("%1x", temp & 0xf);
		if (i != 4) {
			printf(":");
		}
	}
	/*
	 printf("%2x:", 0x9f);
	 printf("%2x:", 0x30);
	 printf("%2x:", 0xe2);
	 printf("%x:", 0x06);		//prints space whe upper nibble =0!  will have to send nibble by nibble
	 printf("%2x:", 0x20);
	 */
	uart_putString("?$\r");  //end command

	//re-enable print for lcd
	init_printf(NULL, putCharLCD);
}

void ascii_array_tohex(uint8_t * pArray) {
	uint8_t size = 0, size2, temp1, temp2, temp3;
	uint64_t temp64 = 0;

	hi = 0;
	lo = 0;

	size = strlen(cmd_data); //this will only work if the end of array is null and array is bytes
	size2 = sizeof(cmd_data) / sizeof(cmd_data[0]);	//size of returns number of bytes the array is, must divide by size
	//size = sizeof(pArray);//figure out how big the array is (will send size back to first null char.
	//while (*pArray++ != 0) { //scan through array
	for (int i = 0; i < size; i++) {
		temp1 = cmd_data[i];	//ascii data
		temp2 = charToHexDigit_test(temp1);
		hexvals[i] = temp2;
		card_hex |= (uint64_t) temp2 << (4 * i); //shift each hex up 4 bits -- //for some reason 9<<7 converting to 0xffffffff90000000  must cast 64 bit!!!
	}

	//we have the single 10 hex values - now pack, to 5 bytes - MSB first
	for (i = 0; i < size; i++) {
		if (i == 0) {
			card_id5[0] = (hexvals[0] << 4) | (hexvals[1]);
		} else if (i == 2) {
			card_id5[1] = (hexvals[2] << 4) | (hexvals[3]);
		} else if (i == 4) {
			card_id5[2] = (hexvals[4] << 4) | (hexvals[5]);
		} else if (i == 6) {
			card_id5[3] = (hexvals[6] << 4) | (hexvals[7]);
		} else if (i == 8) {
			card_id5[4] = (hexvals[8] << 4) | (hexvals[9]);
		}

	}

	hi = card_id5[0];
	lo = card_id5[1] << 24 | card_id5[2] << 16 | card_id5[3] << 8 | card_id5[4];

	__asm("NOP");
}

uint32_t get_card_hi_app() {
	return (hi);
}
uint32_t get_card_lo_app() {
	return (lo);
}

/*
 uint32_t hex_to_ascii(uint8_t hi_nibble, uint8_t lo_nibble){
 uint32_t high = hex_to_int(hi_nibble) * 16;
 uint32_t low = hex_to_int(lo_nibble);
 return high+low;
 }
 */

//http://stackoverflow.com/questions/6933039/convert-two-ascii-bytes-in-one-hexadecimal-byte
//DOES NOT WORK FOR A-F.  HAVE TO AND WITH 0X0F AS IT GIVE 0X2A 0X2B... ETC.
uint8_t charToHexDigit_test(uint8_t c) {
	if (c >= 'A')
		return (c - 'A' + 10) & 0x0f;	//if caps hex?
	else
		return (c - '0') & 0x0f;
}
//http://stackoverflow.com/questions/6933039/convert-two-ascii-bytes-in-one-hexadecimal-byte
unsigned char stringToByte(char c[2]) {
	return charToHexDigit(c[0]) * 16 + charToHexDigit(c[1]);
}

void delay(int del) {

	for (int i = 0; i < del; i++) {
		for (int j = 0; j < 2000; j++) {
			__asm("NOP");
		}
	}

}

void reset_cmd_state() {
	cmd_done = 0;
	i = 0;
	j = 0;
	card_hex = 0;
	clear_cmd_val();
	clear_data();
	clear_hex_vals();
	UART1_C2 |= UART_C2_RE_MASK | UART_C2_TE_MASK;

}

void clear_hex_vals() {
	uint8_t i;
	for (i = 0; i < CARD_ID_VALS_SIZE; i++) {
		hexvals[i] = 0;
	}
}

void clear_cmd_val() {
	uint8_t i;
	for (i = 0; i < CMD_VAL_SIZE; i++) {
		cmd_val[i] = 0;
	}
}

void clear_data() {
	uint8_t i;
	for (i = 0; i < CMD_DATA_SIZE; i++) {
		cmd_data[i] = 0;
	}
}

//init uart1
void init_UART() {
	int ubd, temp;

	SIM_SCGC4 |= SIM_SCGC4_UART1_MASK;	//enable clock to uart

#ifdef FRDM_BRD
//RXI = PTE1  ; TXO = PTE0
	SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;
//TXO
	GPIOE_PDDR |= 1 << 0;  //input=0 output=1
	PORTE_PCR0 |= PORT_PCR_MUX(0X03) | PORT_PCR_DSE_MASK; //mux 3 = UART1_TX, drive strength on
//RXI
	GPIOE_PDDR |= 0 << 1;  //input=0 output=1
	PORTE_PCR1 |= PORT_PCR_MUX(0X03) | PORT_PCR_DSE_MASK; //mux 3 = UART1_RX, drive strength on
#else
			SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;
			//TXO
			GPIOE_PDDR |= 1 << 0;//input=0 output=1
			PORTE_PCR0 |= PORT_PCR_MUX(0X03) | PORT_PCR_DSE_MASK;//mux 3 = UART1_TX, drive strength on
			//RXI
			GPIOE_PDDR |= 0 << 1;//input=0 output=1
			PORTE_PCR1 |= PORT_PCR_MUX(0X03) | PORT_PCR_DSE_MASK;//mux 3 = UART1_RX, drive strength on
#endif

// Calculate baud settings
	ubd = (uint16_t) ((SystemCoreClock) / (BAUD_RATE * 16));
// Shave off the current value of the UARTx_BDH except for the SBR
	UART1_BDH |= UART_BDH_SBR((ubd & 0x1F00) >> 8);
	UART1_BDL = (uint8_t) (ubd & UART_BDL_SBR_MASK);

//SETUP CONTROL REGSITERS - 8N1, RX_ISR
//UART1_C2 |= UART_C2_TCIE_MASK | UART_C2_RIE_MASK; //tx/rx intenrrupt enable

//can setup fifo of size up to 128 words.....uart_pfifo

#ifdef UART_INTERRUPT
	UART1_C2 |= UART_C2_RIE_MASK;
	NVIC_EnableIRQ(UART1_RX_TX_IRQn);		//UART1_IRQn for smaller devices
#endif

//enable transmitter//receiver
	UART1_C2 |= UART_C2_TE_MASK | UART_C2_RE_MASK;

}

//!wrapper to allow printf to output to the lcd without modifying the function structure.
void putc_uart_printf(void* p, char c) {
	uart_putc(c);
}

void uart_putc(uint8_t c) {
	while (!(UART1_S1 & UART_S1_TC_MASK))
		;
	UART1_D = c;
}

int8_t uart_putString(uint8_t *ptr) {
	uint32_t count = 0;
	while (*ptr != 0) {
		while (!(UART1_S1 & UART_S1_TDRE_MASK)) {
			count++;
			if (count > 10000) {
				return (-1);
			}
		}
		uart_putc(*ptr++);
	}
	return(1);
}

