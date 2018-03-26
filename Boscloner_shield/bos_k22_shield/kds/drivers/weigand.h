/*
 * weigand.h
 *
 *  Created on: Dec 12, 2015
 *      Author: MJ
 */

#ifndef SOURCES_WEIGAND_H_
#define SOURCES_WEIGAND_H_

//void PORTC_IRQHandler(void);
void PORTB_IRQHandler(void);
//void PORTE_IRQHandler(void);

extern volatile uint32_t wcount;
extern volatile uint8_t wdone, wstarted;
extern volatile uint8_t data[32];
extern volatile uint8_t card_id5[5];


#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

void InitWeigand();
uint8_t WiegandDone();
uint32_t * get_card_id5();
void resetWeigand();


#endif /* SOURCES_WEIGAND_H_ */
