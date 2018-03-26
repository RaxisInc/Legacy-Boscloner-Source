/*
 * uart_bm.h
 *
 *  Created on: Dec 21, 2015
 *      Author: MJ
 */

#ifndef SOURCES_UART_BM_H_
#define SOURCES_UART_BM_H_


extern volatile uint8_t clone_status;

extern volatile uint8_t cmd_done;
extern volatile uint8_t cmd_data[];

void uart_putc(uint8_t c);
int8_t uart_putString(uint8_t *ptr);
//!wrapper to allow printf to output to the lcd without modifying the function structure.
void putc_uart_printf ( void* p, char c);
uint32_t get_card_lo_app();
uint32_t get_card_hi_app();
void send_app_card_clone();
void send_app_card_scan();
void cmd_process();
void init_UART();
void reset_cmd_state();


#endif /* SOURCES_UART_BM_H_ */
