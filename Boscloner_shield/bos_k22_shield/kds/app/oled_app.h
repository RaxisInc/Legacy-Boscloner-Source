/*
 * oled_app.h
 *
 *  Created on: Dec 15, 2015
 *      Author: MJ
 */

#ifndef SOURCES_OLED_APP_H_
#define SOURCES_OLED_APP_H_


void printRecivedCardApp(uint8_t *pArray);
void print_clone_status(uint8_t * pArray);
void printRecivedCardWiegand(uint8_t *pArray);

void init_disp_update();

void oled_test();
void digitalWrite(uint8_t pin, uint8_t dir);
void pinMode(uint8_t pin, uint8_t dir);


#endif /* SOURCES_OLED_APP_H_ */
