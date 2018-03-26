/* Notes
 * Known issue.  The RX buffer is always 0 in first spi read.  Tried to eliminate by reading 2 time, etc.  still not working correctly
 * - fix: must offset the rx buffer by +1 in order to read correct data.
 */

#include "boscloner.h"
#include "util.h"
#include "proxmark3.h"
#include "at91sam7s512.h"
#include <stdint.h>

#include "apps.h"

#define ENABLE_CLONE 1

#define SPI_TIMEOUT_COUNT 1000

#define CLONE_ID_CMD 	(0Xa1)
#define PM3_ACK 		(0Xb1)
#define SPI_DUMMY_DAT	(0x00)
#define PM3_CLONE_OK  	10
#define PM3_CLONE_ERROR  	11

#define BOS_TRANSFER_SIZE 16
#define BOS_ID_OFFSET 2
#define CLONE_ID_LEN 	5		//10 BYTES OF CLONE DATA

int junk = 0;

uint32_t copy_count = 0;
uint8_t copy_run = 0;
uint8_t clone_run = 0;
uint8_t flag_clone = 0;

bos_clone_state_t bos_clone_state = WAIT;
uint8_t pm3_bos_tx[BOS_TRANSFER_SIZE] = { 0 };
uint8_t pm3_bos_rx[BOS_TRANSFER_SIZE] = { 0 };
uint8_t bos_clone_id[CLONE_ID_LEN] = { 0 };

uint8_t bos_clone_status = 0;	//1 when finished correctly
static uint32_t bos_hi2 = 0, bos_hi = 0, bos_lo = 0; //for 5 byte card (10 hex vals) l0 = bytes low 3 bytes, hi = high 2 byte.
//ex id = 0x20 06 e2 30 9f
//hi = 0x20  lo = 0x06e2309f	= correct

void clear_id_vals();
void clear_rxBuff(void);
void boscloner_get_packet();

//void bos_process_packet();
void bos_send_ack();
int8_t send_clone_packet();
void send_clone_packet_orig();
void bos_send_rxbuf();

#define BOS_PRINT_DEBUG

uint32_t get_id_hi() {
	return bos_hi;
}

uint32_t get_id_lo() {
	return bos_lo;
}

void boscloner_tasks(void) {
	//boscloner_get_packet();

	/*
	 copy_count++;
	 if (copy_count >= 3 * 200000 && copy_run == 0) {
	 CopyHIDtoT55x7(0, 0x24, 0x00a74322, 0);	//doesn't work
	 copy_run = 1;
	 }
	 */

	WDT_HIT();
	send_clone_packet();
	//send_clone_packet_orig();

	/*	//STILL DOES NOT RUN LOCALLY
	 //bos_process_packet();
	 if (flag_clone) {
	 //DbpString("BOSLONE CLONING...\r\n");
	 flag_clone = 0;
	 //void CopyHIDtoT55x7(uint32_t hi2, uint32_t hi, uint32_t lo, uint8_t longFMT)
	 CopyHIDtoT55x7(0, 0x24, 0x00a74321, 0);//doesn't work
	 //CopyHIDtoT55x7(0x18, 0x1, 0x111df7, 0);//trying what is printed as being sent when run from cmd line
	 clone_run = 1;	//only run once for now.
	 }
	 */
}

//get bos shield cmd/data and return ack if valid
int8_t send_clone_packet() {
	uint8_t i = 0;
	uint8_t count = 0;
	//uint8_t timeout = 0;
	//uint8_t send_ack = 0,
	WDT_HIT();
	boscloner_setupspi();
	clear_rxBuff();
	//LED_B_OFF();

	//RECEIVE THE PACKET OF DATA
	for (i = 0; i < BOS_TRANSFER_SIZE; i++) {
		while ((AT91C_BASE_SPI->SPI_SR & AT91C_SPI_TXEMPTY) == 0) {		//TDRE
			count++;	// wait for the transfer to complete
			if (count >= SPI_TIMEOUT_COUNT) {
				Dbprintf("SPI TIMEOUT - SPI_TXEMPTY!");
				return 1;
			}
		}
		AT91C_BASE_SPI->SPI_TDR = 0;	//load data to tx  DUMMY DATA
		count = 0;
		while ((AT91C_BASE_SPI->SPI_SR & AT91C_SPI_ENDTX) == 0) {//ENDTX wait for the transfer to complete
			count++;
			if (count >= SPI_TIMEOUT_COUNT) {
				//Dbprintf("SPI TIMEOUT - spi endtx!");
				return 2;
			}
		}
		while ((AT91C_BASE_SPI->SPI_SR & AT91C_SPI_ENDRX) == 0)
			; //CAN DO A CHECK TO SEE IF THE RX DATA IS VALID = CHECK THE RXBUFF OR ENDRX
		pm3_bos_rx[i] = AT91C_BASE_SPI->SPI_RDR;	//read the data recieved
		//LED_C_INV(); //toggle ledc

		if (i == 0) // the first byte read from buffer is junk data.  needs to second byte after this things are synced = didn't help
			//pm3_bos_rx[i] = AT91C_BASE_SPI->SPI_RDR;	//read the data recieved
			WDT_HIT();
	}			//FOR

	SpinDelayUs(100);	//let slave prepare for next packet.

	//check for correct command - clone cmd recieved
	if (pm3_bos_rx[1] == 0xa1) {	//offset by 1 error - see notes
		clear_id_vals();  //clear last id values
		for (i = 0; i < BOS_TRANSFER_SIZE - BOS_ID_OFFSET; i++) {
			bos_clone_id[i] = pm3_bos_rx[i + BOS_ID_OFFSET];

		}	//for
		bos_send_ack();
		//bos_send_rxbuf();	//debug show raw buffer data
#ifdef ENABLE_CLONE
		flag_clone = 1;
#endif
	}


	//WORKING!!!
	//NOW PACKEAGE HI,LO VALUES FOR CLONING THE CARD
	//hi=0x20  lo=0x06e2309f
	bos_hi = 0;
	bos_lo = 0;
	for (i = 0; i < CLONE_ID_LEN; i++) {
		if (i == 0) {
			bos_hi = bos_clone_id[i];
		} else if (i == 1) {
			bos_lo |= bos_clone_id[i] <<24;
		} else if (i == 2) {
			bos_lo |= bos_clone_id[i] << 16;
		} else if (i == 3) {
			bos_lo |= bos_clone_id[i] << 8;
		} else if (i == 4) {
			bos_lo |= bos_clone_id[i];
		}
	}

	WDT_HIT();
	return (0);	//correct exit

}

void bos_send_ack() {
	uint8_t i = 0;

//boscloner_setupspi();
	for (i = 0; i < BOS_TRANSFER_SIZE; i++) {
		while ((AT91C_BASE_SPI->SPI_SR & AT91C_SPI_TXEMPTY) == 0)
			;	// wait for the transfer to complete
		if (i == 0) {
			AT91C_BASE_SPI->SPI_TDR = PM3_ACK;	//send the ack byte
		} else if (i > 0) {
			AT91C_BASE_SPI->SPI_TDR = pm3_bos_rx[i + 1];//offset by 1 error + skip the cmd value = the recieved data values, +1 effectively is +2 here
		}
	}	//for
}	//function

//this will show the issue of index0=0, then ok data.
void bos_send_rxbuf() {
	uint8_t i = 0;

//boscloner_setupspi();
	for (i = 0; i < BOS_TRANSFER_SIZE; i++) {
		while ((AT91C_BASE_SPI->SPI_SR & AT91C_SPI_TXEMPTY) == 0)
			;	// wait for the transfer to complete
		AT91C_BASE_SPI->SPI_TDR = pm3_bos_rx[i];	//send the rx packet back
	}	//for
}	//function

//get bos shield cmd/data and return ack if valid
//
void send_clone_packet_orig() {
	uint8_t send_ack = 0, i = 0;

	boscloner_setupspi();
	clear_rxBuff();

//RECEIVE THE PACKET OF DATA
	for (i = 0; i < BOS_TRANSFER_SIZE; i++) {
		while ((AT91C_BASE_SPI->SPI_SR & AT91C_SPI_TXEMPTY) == 0)
			;	// wait for the transfer to complete
		AT91C_BASE_SPI->SPI_TDR = SPI_DUMMY_DAT;	//load dummy data
		while ((AT91C_BASE_SPI->SPI_SR & AT91C_SPI_RDRF) == 0)
			;	//read register - wait for data
		pm3_bos_rx[i] = AT91C_BASE_SPI->SPI_RDR;	//read the data recieved
		//LED_C_INV(); //toggle ledc
	}		//FOR

//check if valid command received
	if (pm3_bos_rx[0] == CLONE_ID_CMD) {
		send_ack = 1;
	} else {
		send_ack = 0;
	}

	SpinDelayUs(100);	//let bos setup to recieve
//SEND ACK PACKET - always send the packet, but send 0's if no valid data recieved
	for (i = 0; i < BOS_TRANSFER_SIZE; i++) {
		while ((AT91C_BASE_SPI->SPI_SR & AT91C_SPI_TXEMPTY) == 0)
			;	// wait for the transfer to complete

		if (i == 0 && send_ack == 1) { 	//check cmd data if valid send ack){
			AT91C_BASE_SPI->SPI_TDR = PM3_ACK;	//send the ack byte
		} else if (i > 0 && send_ack == 1) {
			AT91C_BASE_SPI->SPI_TDR = pm3_bos_rx[i];//send the rest of the packet data for print.
		} else {
			AT91C_BASE_SPI->SPI_TDR = 0;	//send 0's if no ack
		}
	}	//FOR

}

void clear_id_vals() {
	uint8_t i = 0;

	bos_hi2 = 0;
	bos_hi = 0;
	bos_lo = 0;
	for (i = 0; i < CLONE_ID_LEN; i++) {
		bos_clone_id[i] = 0;
	}

}

void clear_rxBuff(void) {
	uint8_t i;
	for (i = 0; i < BOS_TRANSFER_SIZE; i++) {
		pm3_bos_rx[i] = 0;
	}

}	//function

/*
 void bos_blink(int count) {
 int i = 0;
 for (i = 0; i < count; i++) {
 LED_B_ON();
 SpinDelay(100);
 LED_B_OFF();
 SpinDelay(100);
 }
 }
 */

/*
 void boscloner_send_spi() {

 uint8_t i, temp = 1;
 uint8_t array_match = 0;

 temp = temp + 1;
 boscloner_setupspi();

 for (i = 0; i < BOS_TRANSFER_SIZE; i++) {

 while ((AT91C_BASE_SPI->SPI_SR & AT91C_SPI_TXEMPTY) == 0)
 // wait for the transfer to complete
 ;
 AT91C_BASE_SPI->SPI_TDR =;	//tx data register
 while ((AT91C_BASE_SPI->SPI_SR & AT91C_SPI_RDRF) == 0)
 ;	//read register - wait for data
 pm3_bos_rx[i] = AT91C_BASE_SPI->SPI_RDR;

 LED_C_INV(); //toggle ledc
 }

 //FAILS
 //test the recieve array for match of sent
 for (i = 0; i < BOS_TRANSFER_SIZE; i++) {
 if (pm3_bos_rx[i] == i) {
 array_match = 1;
 } else {
 array_match = 0;
 break;
 }
 }

 if (array_match == 1) {
 //bos_blink(2);
 LED_B_ON();
 } else {
 //bos_blink(3);
 LED_B_OFF();
 }

 }
 */

void boscloner_setupspi(void) {

// PA10 -> SPI_NCS2 chip select (LCD)
// PA11 -> SPI_NCS0 chip select (FPGA)
// PA12 -> SPI_MISO Master-In Slave-Out
// PA13 -> SPI_MOSI Master-Out Slave-In
// PA14 -> SPI_SPCK Serial Clock

// Disable PIO control of the following pins, allows use by the SPI peripheral
	AT91C_BASE_PIOA->PIO_PDR = GPIO_NCS0 | GPIO_NCS2 | GPIO_MISO | GPIO_MOSI
			| GPIO_SPCK;

	AT91C_BASE_PIOA->PIO_ASR =
	GPIO_NCS0 | GPIO_MISO | GPIO_MOSI | GPIO_SPCK;

	AT91C_BASE_PIOA->PIO_BSR = GPIO_NCS2;

//enable the SPI Peripheral clock
	AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_SPI);
// Enable SPI
	AT91C_BASE_SPI->SPI_CR = AT91C_SPI_SPIEN;

	AT91C_BASE_SPI->SPI_MR = (0 << 24) | // Delay between chip selects (take default: 6 MCK periods)
			(11 << 16) | // Peripheral Chip Select (selects LCD SPI_NCS2 or PA10)
			(0 << 7) |	// Local Loopback Disabled
			(1 << 4) |	// Mode Fault Detection disabled
			(0 << 2) |	// Chip selects connected directly to peripheral
			(0 << 1) |	// Fixed Peripheral Select
			(1 << 0);		// Master Mode
	AT91C_BASE_SPI->SPI_CSR[2] = (1 << 24) |// Delay between Consecutive Transfers (32 MCK periods)
			(1 << 16) |	// Delay Before SPCK (1 MCK period)
			//(6 << 8) |	// Serial Clock Baud Rate (baudrate = MCK/6 = 24Mhz/6 = 4M baud = 8 bits - 500khz = 24Mhz/48
			(49 << 8) |// Serial Clock Baud Rate (baudrate = MCK/6 = 24Mhz/6 = 4M baud = 8 bits - 500khz = (24Mhz/48)+1
			//(16 << 8) |	// Serial Clock Baud Rate (baudrate = MCK/6 = 24Mhz/6 = 4M baud = 8 bits - 500khz = 24Mhz/48
			//( 1 << 4)	|	// Bits per Transfer (9 bits)
			(0 << 4) |  // 8 BITS PER TRANSFER (THE FIELD TO 0)
			(0 << 3) |	// Chip Select inactive after transfer
			(1 << 1) |	// Clock Phase data captured on leading edge, changes on following edge
			(0 << 0);		// Clock Polarity inactive state is logic 0

}

void boslconer_heartbeat(void) {
//HEARTBEAT
	LED_B_ON();
	SpinDelay(10);
	LED_B_OFF();
	SpinDelay(10);

}

//CURRENT SEND RX SPI COMMANDS

//SENDING COMMANDS TO FPGA
/*
 //-----------------------------------------------------------------------------
 // Send a 16 bit command/data pair to the FPGA.
 // The bit format is:  C3 C2 C1 C0 D11 D10 D9 D8 D7 D6 D5 D4 D3 D2 D1 D0
 // where C is the 4 bit command and D is the 12 bit data
 //-----------------------------------------------------------------------------
 void FpgaSendCommand(uint16_t cmd, uint16_t v)
 {
 SetupSpi(SPI_FPGA_MODE);
 while ((AT91C_BASE_SPI->SPI_SR & AT91C_SPI_TXEMPTY) == 0);		// wait for the transfer to complete
 AT91C_BASE_SPI->SPI_TDR = AT91C_SPI_LASTXFER | cmd | v;		// send the data
 }


 //LCD USING SPI TO SEND COMMAND
 void LCDSend(unsigned int data)
 {
 // 9th bit set for data, clear for command
 while ((AT91C_BASE_SPI->SPI_SR & AT91C_SPI_TXEMPTY) == 0);	// wait for the transfer to complete
 // For clarity's sake we pass data with 9th bit clear and commands with 9th
 // bit set since they're implemented as defines, se we need to invert bit
 AT91C_BASE_SPI->SPI_TDR = data^0x100;							// Send the data/command
 }




 */

