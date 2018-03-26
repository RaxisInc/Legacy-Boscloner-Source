#include "MK22F51212.h"
#include "boscloner_app.h"
#include "weigand.h"
#include "Timers.h"
//#include "Cpu.h"


void getCardNumAndSiteCode();
void getCardValues();



uint32_t get_facility_code() ;
uint32_t get_card_code() ;
uint32_t get_card_hex1() ;
uint32_t get_card_hex2() ;
void wiegand_process() ;
void wiegand_test() ;
//uint32_t * get_card_id5() ;
uint8_t WiegandDone() ;
void pack_card_id5() ;
void pack_card_id5_msb() ;
void resetWeigand() ;
void clearLastCardData() ;
void InitWeigand() ;
void InitWeigandPins(void) ;
void InitSysTimers(void) ;



//#define FRDM_BRD_WIEGAND
#define BOS_BRD_WIEGAND

#ifdef FRDM_BRD_WIEGAND
#define WEIGAND_D0_PCR    PORTB_PCR0
#define WEIGAND_D1_PCR    PORTB_PCR1
#endif
#ifdef BOS_BRD_WIEGAND
#define WEIGAND_D0_PCR    PORTC_PCR10
#define WEIGAND_D1_PCR    PORTC_PCR11
#endif

//ON FREEDOM BOARD USING E1/E0
#define WD0_PIN 	PTB0
#define WD0_PIN_LOC 0

#define WD1_PIN 	PTB1	//PTD11 ON THE SHIELD BOARD
#define WD1_PIN_LOC 1

#define DATA_LEN 64

volatile uint32_t wcount = 0;
volatile uint8_t wdone = 0, wstarted = 0;
volatile uint8_t myData[DATA_LEN] = { 0 };

#define TIMEOUT_MS  100		//t5577 takes about 80ms

#define CORE_CLOCK (DEFAULT_SYSTEM_CLOCK/2)
#define SYSTICK_FREQUENCY (1000)

uint32_t temp1, temp2;
volatile uint32_t startTick = 0, endTick = 0;
volatile uint8_t oneHit = 0, zeroHit = 0, doubleHit = 0, doubleHitn = 0;

//ARDUINO BITS
unsigned char databits[DATA_LEN];    // stores all of the data bits
volatile unsigned int bitCount = 0;
unsigned char flagDone;        // goes low when data is currently being captured
unsigned int weigand_counter; // countdown until we assume there are no more bits

volatile unsigned long facilityCode = 0;        // decoded facility code
volatile unsigned long cardCode = 0;            // decoded card code

// Breaking up card value into 2 chunks to create 10 char HEX value
volatile unsigned long bitHolder1 = 0;
volatile unsigned long bitHolder2 = 0;
volatile unsigned long cardChunk1 = 0;
volatile unsigned long cardChunk2 = 0;

volatile uint8_t card_id5[5] = { 0 };

#ifdef FRDM_BRD_WIEGAND
void PORTB_IRQHandler(void) {
	/*//WORKS
	 if ((PORTB_PCR0 & PORT_PCR_ISF_MASK) > 0) {
	 PORTB_PCR0 |= PORT_PCR_ISF_MASK;	//write to the flag to clear it.
	 myData[bitCount++]= 0;
	 }
	 else if( (PORTB_PCR1 & PORT_PCR_ISF_MASK) > 0){
	 PORTB_PCR1 |= PORT_PCR_ISF_MASK;	//write to the flag to clear it.
	 myData[bitCount++]= 1;
	 }
	 */

	if (wstarted == 0 && wdone == 0) { //start of new packet
//TimerStart(t1, TIMEOUT_MS);   //starts t1, 1 second interval
		startTick = GetTick();
		wcount = 0;
		wstarted = 1;
	}

	if (wstarted == 1 && wdone == 0) {
		//ZERO INTERRUPT RECIEVED
		if ((PORTB_PCR0 & PORT_PCR_ISF_MASK) > 0) { //PORTB0 ONE RECIEVED
			PORTB_PCR0 |= PORT_PCR_ISF_MASK;//write to the flag to clear it.

			databits[bitCount++] = 0;//store zero (technically does not need to be written.
			if (bitCount < 23) {
				bitHolder1 = bitHolder1 << 1;
			} else {
				bitHolder2 = bitHolder2 << 1;
			}
		}

		//1 INTERRUPT RECIEVED
		if ((PORTB_PCR1 & PORT_PCR_ISF_MASK) > 0) { //PORTB1 interrupt ZERO RECIEVED
			PORTB_PCR1 |= PORT_PCR_ISF_MASK;//write to the flag to clear it.

			databits[bitCount++] = 1;
			if (bitCount < 23) {
				bitHolder1 = bitHolder1 << 1;
				bitHolder1 |= 0X01;	//set the value to 1
			} else {
				bitHolder2 = bitHolder2 << 1;
				bitHolder2 |= 0X01;	//set the value to 1
			}	//bitcount

		}	//interrupt flag

	}	//storing data
	else {	//clear the flags

		PORTB_PCR0 |= PORT_PCR_ISF_MASK;//write to the flag to clear it.
		PORTB_PCR1 |= PORT_PCR_ISF_MASK;//write to the flag to clear it.
	}
}
#endif

#ifdef BOS_BRD_WIEGAND
void PORTC_IRQHandler(void) {
#endif
	/*//WORKS
	 if ((PORTB_PCR0 & PORT_PCR_ISF_MASK) > 0) {
	 PORTB_PCR0 |= PORT_PCR_ISF_MASK;	//write to the flag to clear it.
	 myData[bitCount++]= 0;
	 }
	 else if( (WEIGAND_D1_PCR & PORT_PCR_ISF_MASK) > 0){
	 WEIGAND_D1_PCR |= PORT_PCR_ISF_MASK;	//write to the flag to clear it.
	 myData[bitCount++]= 1;
	 }
	 */

	if (wstarted == 0 && wdone == 0) {	// WEIGAND_D0_PCRrt of new packet
	//TimerStart(t1, TIMEOUT_MS);   //starts t1, 1 second interval
		startTick = GetTick();
		wcount = 0;
		wstarted = 1;
	}

	if (wstarted == 1 && wdone == 0) {
		//ZERO INTERRUPT RECIEVED
		if ((WEIGAND_D0_PCR & PORT_PCR_ISF_MASK) > 0) {	//PORTB0 ONE RECIEVED
			WEIGAND_D0_PCR |= PORT_PCR_ISF_MASK;//write to the flag to clear it.

			databits[bitCount++] = 0;//store zero (technically does not need to be written.
			if (bitCount < 23) {
				bitHolder1 = bitHolder1 << 1;
			} else {
				bitHolder2 = bitHolder2 << 1;
			}
		}

		//1 INTERRUPT RECIEVED
		if ((WEIGAND_D1_PCR & PORT_PCR_ISF_MASK) > 0) {	//PORTB1 interrupt ZERO RECIEVED
			WEIGAND_D1_PCR |= PORT_PCR_ISF_MASK;//write to the flag to clear it.

			databits[bitCount++] = 1;
			if (bitCount < 23) {
				bitHolder1 = bitHolder1 << 1;
				bitHolder1 |= 0X01;	//set the value to 1
			} else {
				bitHolder2 = bitHolder2 << 1;
				bitHolder2 |= 0X01;	//set the value to 1
			}	//bitcount

		}	//interrupt flag

	}	//storing data
	else {	//clear the flags

		WEIGAND_D0_PCR |= PORT_PCR_ISF_MASK;	//write to the flag to clear it.
		WEIGAND_D1_PCR |= PORT_PCR_ISF_MASK;	//write to the flag to clear it.
	}

}	//interrupt

uint32_t get_facility_code() {
	return (facilityCode);
}

uint32_t get_card_code() {
	return (cardCode);
}

uint32_t get_card_hex1() {
	return (cardChunk1);
}

uint32_t get_card_hex2() {
	return (cardChunk2);
}

void wiegand_process() {

	clearLastCardData();
	getCardValues();
	getCardNumAndSiteCode();
	//pack_card_id5();		//index0 = low byte of card
	pack_card_id5_msb();	//index1 = high byte of card

}

//test the wiegand functionality
void wiegand_test() {
	if (WiegandDone() == 1) {
		__asm("NOP");

		getCardValues();
		getCardNumAndSiteCode();
		//pack_card_id5();		//index0 = low byte of card
		pack_card_id5_msb();	//index1 = high byte of card
		resetWeigand();
	}
}

uint32_t * get_card_id5() {
	return &card_id5;
}

uint8_t WiegandDone() {
	uint32_t temp;
	//if(TimerIsDone(t1) && wstarted==1){  //I guess the packets can be variable length thus will do a timeout rather than count
	if (wstarted == 1) {
		endTick = GetTick();
		temp = endTick - startTick;
		if (temp >= TIMEOUT_MS) {
			return 1;
		}
	}
	return 0;
}

//card hex id: 0x:hex5;hex4;hex3;hex2;hex1
//packet with hex5 at id5[0], hex4[1], hex3[2]....
void pack_card_id5() {
	uint8_t i;

	for (i = 0; i < 5; i++) {
		if (i < 3) {
			card_id5[i] = cardChunk2 >> (i * 8);
		}
		if (i == 3) {
			card_id5[i] = cardChunk1;
		}
		if (i == 4) {
			card_id5[i] = cardChunk1 >> 8;
		}
	}	//for
}	//function

//load the card_id5 array with the id values mosty sig byte first (low).
void pack_card_id5_msb() {
	uint8_t i;

	for (i = 0; i < 5; i++) {
		if (i == 0) {
			card_id5[i] = cardChunk1 >> 8;
		}
		else if (i == 1) {
			card_id5[i] = cardChunk1;
		}
		else if (i == 2) {
			card_id5[i] = cardChunk2 >> 16;
		}
		else if (i == 3) {
			card_id5[i] = cardChunk2 >> 8;
		}
		else if (i == 4) {
			card_id5[i] = cardChunk2;
		}

	}	//for
}	//function

void resetWeigand() {
	uint8_t i;
	//initial statemachine
	wdone = 0;	//start again
	wstarted = 0;
	wcount = 0;
	//clearData();
	bitCount = 0;
	bitHolder1 = 0;
	bitHolder2 = 0;

	for (i = 0; i < DATA_LEN; i++) {
		databits[i] = 0;
	}

}

//arduinoport
// cleanup and get ready for the next card
void clearLastCardData() {
	facilityCode = 0;
	cardCode = 0;
	cardChunk1 = 0;
	cardChunk2 = 0;
}

void InitWeigand() {
	InitSysTimers();
	InitWeigandPins();
}

void InitWeigandPins(void) {

	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTC_MASK
			| SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK;

#ifdef FRDM_BRD_WIEGAND
	//PORTD TEST PINd15 = pe1 = WEIGAND D1
	GPIOB_PDDR |= 0 << 1;//Pte1 AS INPUT=0
	PORTB_PCR1 |= PORT_PCR_MUX(1);//mux 1 = gpio
	PORTB_PCR1 |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;//enable pull up
	PORTB_PCR1 |= PORT_PCR_ISF_MASK | PORT_PCR_IRQC(0x09);//interrupt on falling edge = 0xa0, rise=0x09, either 0x0b,clear interrupt mask and setup either direction interrupt

	//PORTD TEST PINd15 = pe1 = WEIGAND D1
	GPIOB_PDDR |= 0 << 0;//Pte1 AS INPUT=0
	PORTB_PCR0 |= PORT_PCR_MUX(1);//mux 1 = gpio
	PORTB_PCR0 |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;//enable pull up
	PORTB_PCR0 |= PORT_PCR_ISF_MASK | PORT_PCR_IRQC(0x09);//interrupt on falling edge = 0xa0, rise=0x09, either 0x0b,clear interrupt mask and setup either direction interrupt
	NVIC_EnableIRQ(PORTB_IRQn);
#endif
#ifdef BOS_BRD_WIEGAND
	//ptc10 = d0 , ptc11 = d1
	GPIOC_PDDR |= 10 << 1;  //PtC10 AS INPUT=0
	WEIGAND_D0_PCR |= PORT_PCR_MUX(1);  //mux 1 = gpio
	WEIGAND_D0_PCR |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;	//enable pull up
	WEIGAND_D0_PCR |= PORT_PCR_ISF_MASK | PORT_PCR_IRQC(0x09);//interrupt on falling edge = 0xa0, rise=0x09, either 0x0b,clear interrupt mask and setup either direction interrupt

	GPIOC_PDDR |= 11 << 1;  //PtC10 AS INPUT=0
	WEIGAND_D1_PCR |= PORT_PCR_MUX(1);  //mux 1 = gpio
	WEIGAND_D1_PCR |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;	//enable pull up
	WEIGAND_D1_PCR |= PORT_PCR_ISF_MASK | PORT_PCR_IRQC(0x09);//interrupt on falling edge = 0xa0, rise=0x09, either 0x0b,clear interrupt mask and setup either direction interrupt
	NVIC_EnableIRQ(PORTC_IRQn);
#endif

}

void InitSysTimers(void) {
	uint8_t tmp;

	InitTimers();
	t1 = RegisterTimer();

	tmp = SysTick_Config(SystemCoreClock / (SYSTICK_FREQUENCY));
	//NVIC_EnableIRQ(SysTick_IRQn);//not needed

}

void clearData(void) {
	uint32_t i = 0;
	for (i = 0; i < DATA_LEN; i++) {
		//	data[i] = 0;
	}

}

///////////////////////////////////////////////////////
// SETUP function
void getCardNumAndSiteCode() {
	unsigned char i;

	// we will decode the bits differently depending on how many bits we have
	// see www.pagemac.com/azure/data_formats.php for more info
	// also specifically: www.brivo.com/app/static_data/js/calculate.js
	switch (bitCount) {

	///////////////////////////////////////
	// standard 26 bit format
	// facility code = bits 2 to 9
	case 26:
		for (i = 1; i < 9; i++) {
			facilityCode <<= 1;
			facilityCode |= databits[i];
		}

		// card code = bits 10 to 23
		for (i = 9; i < 25; i++) {
			cardCode <<= 1;
			cardCode |= databits[i];
		}
		break;

		///////////////////////////////////////
		// 33 bit HID Generic
	case 33:
		for (i = 1; i < 8; i++) {
			facilityCode <<= 1;
			facilityCode |= databits[i];
		}

		// card code
		for (i = 8; i < 32; i++) {
			cardCode <<= 1;
			cardCode |= databits[i];
		}
		break;

		///////////////////////////////////////
		// 34 bit HID Generic
	case 34:
		for (i = 1; i < 17; i++) {
			facilityCode <<= 1;
			facilityCode |= databits[i];
		}

		// card code
		for (i = 17; i < 33; i++) {
			cardCode <<= 1;
			cardCode |= databits[i];
		}
		break;

		///////////////////////////////////////
		// 35 bit HID Corporate 1000 format
		// facility code = bits 2 to 14
	case 35:
		for (i = 2; i < 14; i++) {
			facilityCode <<= 1;
			facilityCode |= databits[i];
		}

		// card code = bits 15 to 34
		for (i = 14; i < 34; i++) {
			cardCode <<= 1;
			cardCode |= databits[i];
		}
		break;

	}
	return;
}

//////////////////////////////////////
// Function to append the card value (bitHolder1 and bitHolder2) to the necessary array then tranlate that to
// the two chunks for the card value that will be output
void getCardValues() {
	switch (bitCount) {
	case 26:
		// Example of full card value
		// |>   preamble   <| |>   Actual card value   <|
		// 000000100000000001 11 111000100000100100111000
		// |> write to chunk1 <| |>  write to chunk2   <|

		for (int i = 19; i >= 0; i--) {
			if (i == 13 || i == 2) {
				bitWrite(cardChunk1, i, 1); // Write preamble 1's to the 13th and 2nd bits
			} else if (i > 2) {
				bitWrite(cardChunk1, i, 0); // Write preamble 0's to all other bits above 1
			} else {
				bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 20)); // Write remaining bits to cardChunk1 from bitHolder1
			}
			if (i < 20) {
				bitWrite(cardChunk2, i + 4, bitRead(bitHolder1, i)); // Write the remaining bits of bitHolder1 to cardChunk2
			}
			if (i < 4) {
				bitWrite(cardChunk2, i, bitRead(bitHolder2, i)); // Write the remaining bit of cardChunk2 with bitHolder2 bits
			}
		}
		break;

	case 27:
		for (int i = 19; i >= 0; i--) {
			if (i == 13 || i == 3) {
				bitWrite(cardChunk1, i, 1);
			} else if (i > 3) {
				bitWrite(cardChunk1, i, 0);
			} else {
				bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 19));
			}
			if (i < 19) {
				bitWrite(cardChunk2, i + 5, bitRead(bitHolder1, i));
			}
			if (i < 5) {
				bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
			}
		}
		break;

	case 28:
		for (int i = 19; i >= 0; i--) {
			if (i == 13 || i == 4) {
				bitWrite(cardChunk1, i, 1);
			} else if (i > 4) {
				bitWrite(cardChunk1, i, 0);
			} else {
				bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 18));
			}
			if (i < 18) {
				bitWrite(cardChunk2, i + 6, bitRead(bitHolder1, i));
			}
			if (i < 6) {
				bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
			}
		}
		break;

	case 29:
		for (int i = 19; i >= 0; i--) {
			if (i == 13 || i == 5) {
				bitWrite(cardChunk1, i, 1);
			} else if (i > 5) {
				bitWrite(cardChunk1, i, 0);
			} else {
				bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 17));
			}
			if (i < 17) {
				bitWrite(cardChunk2, i + 7, bitRead(bitHolder1, i));
			}
			if (i < 7) {
				bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
			}
		}
		break;

	case 30:
		for (int i = 19; i >= 0; i--) {
			if (i == 13 || i == 6) {
				bitWrite(cardChunk1, i, 1);
			} else if (i > 6) {
				bitWrite(cardChunk1, i, 0);
			} else {
				bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 16));
			}
			if (i < 16) {
				bitWrite(cardChunk2, i + 8, bitRead(bitHolder1, i));
			}
			if (i < 8) {
				bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
			}
		}
		break;

	case 31:
		for (int i = 19; i >= 0; i--) {
			if (i == 13 || i == 7) {
				bitWrite(cardChunk1, i, 1);
			} else if (i > 7) {
				bitWrite(cardChunk1, i, 0);
			} else {
				bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 15));
			}
			if (i < 15) {
				bitWrite(cardChunk2, i + 9, bitRead(bitHolder1, i));
			}
			if (i < 9) {
				bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
			}
		}
		break;

	case 32:
		for (int i = 19; i >= 0; i--) {
			if (i == 13 || i == 8) {
				bitWrite(cardChunk1, i, 1);
			} else if (i > 8) {
				bitWrite(cardChunk1, i, 0);
			} else {
				bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 14));
			}
			if (i < 14) {
				bitWrite(cardChunk2, i + 10, bitRead(bitHolder1, i));
			}
			if (i < 10) {
				bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
			}
		}
		break;

	case 33:
		for (int i = 19; i >= 0; i--) {
			if (i == 13 || i == 9) {
				bitWrite(cardChunk1, i, 1);
			} else if (i > 9) {
				bitWrite(cardChunk1, i, 0);
			} else {
				bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 13));
			}
			if (i < 13) {
				bitWrite(cardChunk2, i + 11, bitRead(bitHolder1, i));
			}
			if (i < 11) {
				bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
			}
		}
		break;

	case 34:
		for (int i = 19; i >= 0; i--) {
			if (i == 13 || i == 10) {
				bitWrite(cardChunk1, i, 1);
			} else if (i > 10) {
				bitWrite(cardChunk1, i, 0);
			} else {
				bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 12));
			}
			if (i < 12) {
				bitWrite(cardChunk2, i + 12, bitRead(bitHolder1, i));
			}
			if (i < 12) {
				bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
			}
		}
		break;

	case 35:
		for (int i = 19; i >= 0; i--) {
			if (i == 13 || i == 11) {
				bitWrite(cardChunk1, i, 1);
			} else if (i > 11) {
				bitWrite(cardChunk1, i, 0);
			} else {
				bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 11));
			}
			if (i < 11) {
				bitWrite(cardChunk2, i + 13, bitRead(bitHolder1, i));
			}
			if (i < 13) {
				bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
			}
		}
		break;

	case 36:
		for (int i = 19; i >= 0; i--) {
			if (i == 13 || i == 12) {
				bitWrite(cardChunk1, i, 1);
			} else if (i > 12) {
				bitWrite(cardChunk1, i, 0);
			} else {
				bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 10));
			}
			if (i < 10) {
				bitWrite(cardChunk2, i + 14, bitRead(bitHolder1, i));
			}
			if (i < 14) {
				bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
			}
		}
		break;

	case 37:
		for (int i = 19; i >= 0; i--) {
			if (i == 13) {
				bitWrite(cardChunk1, i, 0);
			} else {
				bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 9));
			}
			if (i < 9) {
				bitWrite(cardChunk2, i + 15, bitRead(bitHolder1, i));
			}
			if (i < 15) {
				bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
			}
		}
		break;
	}
	return;
}

//PORTE0/1 SETUP:
/*
 uint32_t temp1, temp2;
 void PORTE_IRQHandler(void){



 if( (PORTE_PCR0 &PORT_PCR_ISF_MASK) >= 1){	//porte0 interrupt
 __asm("NOP");
 PORTE_PCR0 |= PORT_PCR_ISF_MASK;	//write to the flag to clear it.
 }


 if( (PORTE_PCR1 &PORT_PCR_ISF_MASK) >= 1){	//porte1 interrupt
 __asm("NOP");
 PORTE_PCR1 |= PORT_PCR_ISF_MASK;	//write to the flag to clear it.
 }


 }


 void InitWeigandPins(void){


 SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK;

 //PORTD TEST PINd15 = pe1 = WEIGAND D1
 GPIOE_PDDR |= 0<<1;  //Pte1 AS INPUT=0
 PORTE_PCR1 |= PORT_PCR_MUX(1) ;  //mux 1 = gpio
 PORTE_PCR1 |=  PORT_PCR_PE_MASK| PORT_PCR_PS_MASK;	//enable pull up
 PORTE_PCR1 |= PORT_PCR_ISF_MASK | PORT_PCR_IRQC(0x09);	//interrupt on falling edge = 0xa0, rise=0x09, either 0x0b,clear interrupt mask and setup either direction interrupt


 //PORTD TEST PINd15 = pe1 = WEIGAND D1
 GPIOE_PDDR |= 0<<0;  //Pte1 AS INPUT=0
 PORTE_PCR0 |= PORT_PCR_MUX(1) ;  //mux 1 = gpio
 PORTE_PCR0 |=  PORT_PCR_PE_MASK| PORT_PCR_PS_MASK;	//enable pull up
 PORTE_PCR0 |= PORT_PCR_ISF_MASK | PORT_PCR_IRQC(0x09);	//interrupt on falling edge = 0xa0, rise=0x09, either 0x0b,clear interrupt mask and setup either direction interrupt
 NVIC_EnableIRQ(PORTE_IRQn);


 //SETUP WD0 = PC10
 GPIOC_PDDR |= 0<<WD0_PIN_LOC;  //PTD1 AS INPUT
 PORTC_PCR10 |= PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK;  //mux 1 = gpio
 PORTC_PCR10 |=  PORT_PCR_PE_MASK| PORT_PCR_PS_MASK;	//enable pull up
 PORTC_PCR10 |= PORT_PCR_IRQC(0x09);  //interrupt on falling edge = 0xa0, rise=0x09, either 0x0b

 //SETUP WD1
 GPIOC_PDDR |= 0<<WD1_PIN_LOC;  //PTD1 AS INPUT
 PORTC_PCR1 |= PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK;

 //void NVIC_EnableIRQ(IRQn_Type IRQn)
 NVIC_EnableIRQ(PORTC_IRQn);

 }
 */

