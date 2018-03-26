/*
 * dspi.c
 *
 *  Created on: Jan 22, 2016
 *      Author: MJ
 */

//#include "main.h"
#include "dspi.h"
#include "boscloner_app.h"

#define DSPI_SLAVE_INSTANCE         1 /*! User change define to choose DSPI instance */
#define TRANSFER_SIZE               (16)                /*! Transfer size */

static dspi_status_t dspiResult;
static dspi_slave_state_t slaveState;
static dspi_slave_user_config_t slaveConfig;

static uint8_t receiveBuffer[TRANSFER_SIZE] = { 0 };
static uint8_t sendBuffer[TRANSFER_SIZE] = { 0 };

#define CLONE_ID_LEN 10
#define CLONE_ID_CMD (0XA1)
#define PM3_ACK (0XB1)

uint64_t over_flow_count = 0;
#define SPI_TIMEOUT_COUNT 100	//typical when running 96mhz
#define SPI_TIMEOUT_I0_COUNT  100000

static uint8_t clone_id[CLONE_ID_LEN] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

uint8_t send_pm3_packet();
uint8_t send_clone_cmd();
uint8_t txrx_spis_blocking_packet();
uint8_t WORKING_MAIN_txrx_spis_blocking_packet();

void SPI1_IRQHandler(void) {	//required for the fsl api to work correctly
	DSPI_DRV_IRQHandler(SPI1_IDX);
}

void init_dspi() {
	PORT_HAL_SetMuxMode(PORTD, 4u, kPortMuxAlt7);	//mux7 = spi funciton
	PORT_HAL_SetMuxMode(PORTD, 5u, kPortMuxAlt7);
	PORT_HAL_SetMuxMode(PORTD, 6u, kPortMuxAlt7);
	PORT_HAL_SetMuxMode(PORTD, 7u, kPortMuxAlt7);

	CLOCK_SYS_EnablePortClock(PORTD_IDX);

	init_dspi_vals();
	setup_dspi_regs();
}

//merging into working code! - works
void test_send_packet4_() {
	//init_send_clone_packet();
	//init_send_bos();	//works perfectly.
	init_send_clone_packet(get_card_id5());
	send_pm3_packet();
	clearRxBuffer();
	send_pm3_packet();
	//txrx_spis_blocking_packet();
	if (receiveBuffer[0] == 0xb1) {
		__asm("NOP");
	}
	clearTxBuffer();
}


//merging into working code! - works
void send_clone_cmd_pm3() {
	//init_send_clone_packet();
	//init_send_bos();	//works perfectly.
	init_send_clone_packet(get_card_id5());
	send_pm3_packet();
	clearRxBuffer();
	send_pm3_packet();
	//txrx_spis_blocking_packet();
	if (receiveBuffer[0] == 0xb1) {
		__asm("NOP");
	}
	clearTxBuffer();
}



//BLOCKING SPI FUCNTION TO SEND BOSCLONE/PM3 DATA PACKET
uint8_t send_pm3_packet() {
	char temp;
	uint8_t index;
	uint32_t count = 0;

	setup_dspi_regs();
	unhalt_dspi();

	for (index = 0; index < TRANSFER_SIZE; index++) {
		while (!SPI1_SR & SPI_SR_TFFF_MASK)
			;	//wait for fill flag
		SPI1_PUSHR_SLAVE = sendBuffer[index];
		//SPI1_PUSHR_SLAVE = index;
		while (!(SPI1_SR & SPI_SR_TCF_MASK)) {//wait for transmit complete flag
			count++;
			if (count >= SPI_TIMEOUT_COUNT && index != 0) {	//TIMOUT RESET AND EXIT
				clear_fifos();
				over_flow_count = 0;
				//init_dspi_slavefromEx();//this is the only way to get next frame to output correctly at high speed.
				setup_dspi_regs();
				halt_dspi();
				return 1;//when it returns there is still a send in the buffer, need to clear buffer here?
			} /*else if (count >= SPI_TIMEOUT_I0_COUNT && index == 0) {
				clear_fifos();
				over_flow_count = 0;
				//init_dspi_slavefromEx();//this is the only way to get next frame to output correctly at high speed.
				setup_dspi_regs();
				halt_dspi();
				return 1;//when it returns there is still a send in the buffer, need to clear buffer here?
			}*/
		}
		count = 0;
		receiveBuffer[index] = SPI1_POPR;
		SPI1_SR = 0XFFFFFFFF;	// clear all flags
	}
	//DSPI_HAL_WriteDataSlavemodeBlocking(SPI1, c);	//works by itself
	//init_dspi_slavefromEx();
	setup_dspi_regs();
	halt_dspi();
	over_flow_count++;
	clear_fifos();

	if (receiveBuffer[0] == 0xb1 || receiveBuffer[0] != 0) {	//
		__asm("NOP");
	}
	//SPI1_MCR
	__asm("NOP");
}


//WORKING FROM UART BM CODE PROJECT
#define SPI_TIMEOUT_COUNT 100	//typical when running 96mhz
uint8_t WORKING_MAIN_txrx_spis_blocking_packet() {
	char temp;
	uint8_t index;
	uint32_t count = 0;

	setup_dspi_regs();
	unhalt_dspi();

	for (index = 0; index < TRANSFER_SIZE; index++) {
		while (!SPI1_SR & SPI_SR_TFFF_MASK)
			;	//wait for fill flag
		SPI1_PUSHR_SLAVE = sendBuffer[index];
		//SPI1_PUSHR_SLAVE = index;
		while (!(SPI1_SR & SPI_SR_TCF_MASK)) {//wait for transmit complete flag
			count++;
			if (count >= SPI_TIMEOUT_COUNT && index != 0) {	//TIMOUT RESET AND EXIT
				clear_fifos();
				over_flow_count = 0;
				init_dspi_slavefromEx();//this is the only way to get next frame to output correctly at high speed.
				return 1;//when it returns there is still a send in the buffer, need to clear buffer here?
			}
		}
		count = 0;
		receiveBuffer[index] = SPI1_POPR;
		SPI1_SR = 0XFFFFFFFF;	// clear all flags
	}
	//DSPI_HAL_WriteDataSlavemodeBlocking(SPI1, c);	//works by itself
	init_dspi_slavefromEx();
	over_flow_count++;
	clear_fifos();

	//SPI1_MCR
	__asm("NOP");
}




#define SPI_TIMEOUT_COUNT 100	//typical when running 96mhz
uint8_t txrx_spis_blocking_packet() {
	char temp;
	uint8_t index;
	uint32_t count = 0;

	restart();

	for (index = 0; index < TRANSFER_SIZE; index++) {
		while (!SPI1_SR & SPI_SR_TFFF_MASK)
			;	//wait for fill flag
		SPI1_PUSHR_SLAVE = sendBuffer[index];
		//SPI1_PUSHR_SLAVE = index;
		while (!(SPI1_SR & SPI_SR_TCF_MASK)) {//wait for transmit complete flag
			count++;
			if (count >= SPI_TIMEOUT_COUNT && index != 0) {	//TIMOUT RESET AND EXIT
				clear_fifos();
				over_flow_count = 0;
				init_dspi_slavefromEx();//this is the only way to get next frame to output correctly at high speed.
				return 1;//when it returns there is still a send in the buffer, need to clear buffer here?
			}
		}
		count = 0;
		receiveBuffer[index] = SPI1_POPR;
		SPI1_SR = 0XFFFFFFFF;	// clear all flags
	}
	//DSPI_HAL_WriteDataSlavemodeBlocking(SPI1, c);	//works by itself
	init_dspi_slavefromEx();
	over_flow_count++;
	clear_fifos();

	//SPI1_MCR
	__asm("NOP");
}

void restart() {
	SPI1_MCR &= ~SPI_MCR_HALT_MASK;
}

void unhalt_dspi() {
	SPI1_MCR &= ~SPI_MCR_HALT_MASK;
}

void halt_dspi() {
	SPI1_MCR |= SPI_MCR_HALT_MASK;
}

void init_send_clone_packet(uint8_t * pcard_id) {
	uint8_t i;
	for (i = 0; i < TRANSFER_SIZE; i++) {
		if (i == 0)
			sendBuffer[i] = CLONE_ID_CMD;
		else if (i < 6)
			sendBuffer[i] = *pcard_id++;
		else
			sendBuffer[i] = 0;
	}	//for
	asm("NOP");
}

void init_send_bos() {
	uint8_t i;
	for (i = 0; i < TRANSFER_SIZE; i++) {
		if (i == 0) {
			sendBuffer[i] = CLONE_ID_CMD;
		} else
			sendBuffer[i] = i;
	}
}

void clearRxBuffer() {
	int i;
	for (i = 0; i < TRANSFER_SIZE; i++) {
		receiveBuffer[i] = 0;
	}
}

void clearTxBuffer() {
	int i;
	for (i = 0; i < TRANSFER_SIZE; i++) {
		sendBuffer[i] = 0;
	}
}

void clear_fifos() {
	uint8_t junk;

	//junk =SPI1_POPR;	//THIS SEEMED TO HAVE HELPED.  DID NOT CLAEAR FLAG BUT NEXT PACKET DOES NOT START WITH STORED VALUE.

	SPI1_MCR |= SPI_MCR_CLR_RXF_MASK | SPI_MCR_CLR_TXF_MASK;

	SPI1_MCR |= SPI_MCR_HALT_MASK;
	SPI1_SR = 0XFFFFFFFF;	// clear all flags
	SPI1_MCR |= SPI_MCR_MDIS_MASK;
	SPI1_SR = 0XFFFFFFFF;

}

//this gets out of sync with the master on pm3 - why?
uint8_t tx_spis_blocking2(uint8_t c) {
	char temp;
	uint8_t i;

	while (!SPI1_SR & SPI_SR_TFFF_MASK)
		;	//wait for fill flag
	SPI1_PUSHR_SLAVE = c;
	while (!(SPI1_SR & SPI_SR_TCF_MASK))
		;	//wait for transmit complete flag
	temp = SPI1_POPR;

	SPI1_SR = 0XFFFFFFFF;	// clear all flags
	//DSPI_HAL_WriteDataSlavemodeBlocking(SPI1, c);	//works by itself
	__asm("NOP");
}

void init_dspi_vals() {
	slaveState.status = kStatus_DSPI_Success;
	slaveState.errorCount = 0;
	slaveState.dummyPattern = slaveConfig.dummyPattern;
	slaveState.remainingSendByteCount = 0;
	slaveState.remainingReceiveByteCount = 0;
	slaveState.isTransferInProgress = false;
	slaveState.receiveBuffer = NULL;
	slaveState.sendBuffer = NULL;
	slaveState.bitsPerFrame = 8;

	// Setup the configuration
	slaveConfig.dataConfig.bitsPerFrame = 8;
	slaveConfig.dataConfig.clkPhase = kDspiClockPhase_FirstEdge;
	slaveConfig.dataConfig.clkPolarity = kDspiClockPolarity_ActiveHigh;
	slaveConfig.dummyPattern = 0;
}

void setup_dspi_regs() {

//SIM_HAL_EnableClock(SIM, spiGateTable[1]);
	SIM_SCGC6 |= SIM_SCGC6_SPI1_MASK; //enable clock to spi1
	SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK; //enable clock to portd - will cause fault if not.

//reset spi register values to initial values*******************
	SPI_BWR_MCR_MDIS(SPI1, 0);
	/* Halt all transfers*/
	SPI_BWR_MCR_HALT(SPI1, 1);
	/* set the registers to their default states*/
	/* clear the status bits (write-1-to-clear)*/
	SPI_WR_SR(SPI1,
			SPI_SR_TCF_MASK | SPI_SR_EOQF_MASK | SPI_SR_TFUF_MASK | SPI_SR_TFFF_MASK | SPI_SR_RFOF_MASK | SPI_SR_RFDF_MASK);

	SPI_WR_TCR(SPI1, 0);
	SPI_WR_CTAR(SPI1, 0, 0x78000000); /* CTAR0 */
	SPI_WR_CTAR(SPI1, 1, 0x78000000); /* CTAR1 */
	SPI_WR_RSER(SPI1, 0);

	/* Clear out PUSHR register. Since DSPI is halted, nothing should be transmitted. Be
	 * sure the flush the FIFOs afterwards
	 */
	SPI_WR_PUSHR(SPI1, 0);

	/* flush the fifos*/
	SPI_BWR_MCR_CLR_TXF(SPI1, true);
	SPI_BWR_MCR_CLR_RXF(SPI1, true);

	/* Now set MCR to default value, which disables module: set MDIS and HALT, clear other bits */
	SPI_WR_MCR(SPI1, SPI_MCR_MDIS_MASK | SPI_MCR_HALT_MASK);	//

//!/* Set to slave mode.     DSPI_HAL_SetMasterSlaveMode(SPI1, kDspiSlave);
	SPI_BWR_MCR_MSTR(SPI1, 0);

//!DSPI_HAL_SetDataFormat
//set data format
	SPI_BWR_CTAR_FMSZ(SPI1, 0, (slaveConfig.dataConfig.bitsPerFrame - 1));
	SPI_BWR_CTAR_CPOL(SPI1, 0, slaveConfig.dataConfig.clkPolarity);
	SPI_BWR_CTAR_CPHA(SPI1, 0, slaveConfig.dataConfig.clkPhase);
	SPI_BWR_CTAR_LSBFE(SPI1, 0, slaveConfig.dataConfig.direction);

//!/* Enable fifo operation (regardless of FIFO depth) */ DSPI_HAL_SetFifoCmd(SPI1, true, true);
//clear the spi disable bit = enable
	DSPI_HAL_Enable(SPI1);    //= SPI_BWR_MCR_MDIS(SPI1, 0);
	/* Note, the bit definition is "disable FIFO", so a "1" would disable. If user wants to enable
	 * the FIFOs, they pass in true, which we must logically negate (turn to false) to enable the
	 * FIFO
	 */
	SPI_BWR_MCR_DIS_TXF(SPI1, 1);
	SPI_BWR_MCR_DIS_RXF(SPI1, 1);

	SPI_BWR_MCR_MDIS(SPI1, 1);  //DISABLE AGAIN
//! ENABLE THE SPI MODULE AGAIN
	SPI_BWR_MCR_MDIS(SPI1, 0);

	SPI_BWR_MCR_CLR_TXF(SPI1, 1);
	SPI_BWR_MCR_CLR_RXF(SPI1, 1);

//! flush the fifos
#ifdef USE_FIFO
	SPI_BWR_MCR_DIS_TXF(SPI1, 0);
	SPI_BWR_MCR_DIS_RXF(SPI1, 0);
#endif

//! /* Clear the Tx FIFO Fill Flag (TFFF) status bit */
	DSPI_HAL_ClearStatusFlag(SPI1, kDspiTxFifoFillRequest);

#ifdef USE_INTERRUPT
//Analbe iRQ
	SPI_RSER_REG(SPI1) |= SPI_RSER_TCF_RE_MASK;//Enables TCF flag in the SR to generate an interrupt request.
	NVIC_EnableIRQ(27);//27
#endif

//enable freeze for debug mode
	SPI1_MCR |= SPI_MCR_FRZ_MASK;

//! enable module again
	SPI_BWR_MCR_MDIS(SPI1, 0);		//senalbe
	SPI_BWR_MCR_HALT(SPI1, 0);		//start transfer = clear halt
}

void init_dspi_slavefromEx() {
/////!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!DSPI INIT FUNCTION
	slaveState.status = kStatus_DSPI_Success;
	slaveState.errorCount = 0;
	slaveState.dummyPattern = slaveConfig.dummyPattern;
	slaveState.remainingSendByteCount = 0;
	slaveState.remainingReceiveByteCount = 0;
	slaveState.isTransferInProgress = false;
	slaveState.receiveBuffer = NULL;
	slaveState.sendBuffer = NULL;

	slaveState.bitsPerFrame = 8;
//SIM_HAL_EnableClock(SIM, spiGateTable[1]);
	SIM_SCGC6 |= SIM_SCGC6_SPI1_MASK; //enable clock to spi1
	SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK; //enable clock to portd - will cause fault if not.

//reset spi register values to initial values*******************
	SPI_BWR_MCR_MDIS(SPI1, 0);
	/* Halt all transfers*/
	SPI_BWR_MCR_HALT(SPI1, 1);
	/* set the registers to their default states*/
	/* clear the status bits (write-1-to-clear)*/
	SPI_WR_SR(SPI1,
			SPI_SR_TCF_MASK | SPI_SR_EOQF_MASK | SPI_SR_TFUF_MASK | SPI_SR_TFFF_MASK | SPI_SR_RFOF_MASK | SPI_SR_RFDF_MASK);

	SPI_WR_TCR(SPI1, 0);
	SPI_WR_CTAR(SPI1, 0, 0x78000000); /* CTAR0 */
	SPI_WR_CTAR(SPI1, 1, 0x78000000); /* CTAR1 */
	SPI_WR_RSER(SPI1, 0);

	/* Clear out PUSHR register. Since DSPI is halted, nothing should be transmitted. Be
	 * sure the flush the FIFOs afterwards
	 */
	SPI_WR_PUSHR(SPI1, 0);

	/* flush the fifos*/
	SPI_BWR_MCR_CLR_TXF(SPI1, true);
	SPI_BWR_MCR_CLR_RXF(SPI1, true);

	/* Now set MCR to default value, which disables module: set MDIS and HALT, clear other bits */
	SPI_WR_MCR(SPI1, SPI_MCR_MDIS_MASK | SPI_MCR_HALT_MASK);	//

//!/* Set to slave mode.     DSPI_HAL_SetMasterSlaveMode(SPI1, kDspiSlave);
	SPI_BWR_MCR_MSTR(SPI1, 0);

//!DSPI_HAL_SetDataFormat
//set data format
	SPI_BWR_CTAR_FMSZ(SPI1, 0, (slaveConfig.dataConfig.bitsPerFrame - 1));
	SPI_BWR_CTAR_CPOL(SPI1, 0, slaveConfig.dataConfig.clkPolarity);
	SPI_BWR_CTAR_CPHA(SPI1, 0, slaveConfig.dataConfig.clkPhase);
	SPI_BWR_CTAR_LSBFE(SPI1, 0, slaveConfig.dataConfig.direction);

//!/* Enable fifo operation (regardless of FIFO depth) */ DSPI_HAL_SetFifoCmd(SPI1, true, true);
//clear the spi disable bit = enable
	DSPI_HAL_Enable(SPI1);    //= SPI_BWR_MCR_MDIS(SPI1, 0);
	/* Note, the bit definition is "disable FIFO", so a "1" would disable. If user wants to enable
	 * the FIFOs, they pass in true, which we must logically negate (turn to false) to enable the
	 * FIFO
	 */
	SPI_BWR_MCR_DIS_TXF(SPI1, 1);
	SPI_BWR_MCR_DIS_RXF(SPI1, 1);

	SPI_BWR_MCR_MDIS(SPI1, 1);  //DISABLE AGAIN
//! ENABLE THE SPI MODULE AGAIN
	SPI_BWR_MCR_MDIS(SPI1, 0);

	SPI_BWR_MCR_CLR_TXF(SPI1, 1);
	SPI_BWR_MCR_CLR_RXF(SPI1, 1);

//! flush the fifos
#ifdef USE_FIFO
	SPI_BWR_MCR_DIS_TXF(SPI1, 0);
	SPI_BWR_MCR_DIS_RXF(SPI1, 0);
#endif

//! /* Clear the Tx FIFO Fill Flag (TFFF) status bit */
	DSPI_HAL_ClearStatusFlag(SPI1, kDspiTxFifoFillRequest);

#ifdef USE_INTERRUPT
//Analbe iRQ
	SPI_RSER_REG(SPI1) |= SPI_RSER_TCF_RE_MASK;//Enables TCF flag in the SR to generate an interrupt request.
	NVIC_EnableIRQ(27);//27
#endif

//enable freeze for debug mode
	SPI1_MCR |= SPI_MCR_FRZ_MASK;

//! enable module again
	SPI_BWR_MCR_MDIS(SPI1, 0);		//senalbe
	SPI_BWR_MCR_HALT(SPI1, 0);		//start transfer = clear halt

}

