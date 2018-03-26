/*
 * spi_driver.h
 *
 *  Created on: Dec 15, 2015
 *      Author: MJ
 */

#ifndef SOURCES_SPI_DRIVER_H_
#define SOURCES_SPI_DRIVER_H_


#define PIN_C_D_LOC 6
#define PIN_C_D_HIGH  		(GPIOC_PSOR|= 1<<PIN_C_D_LOC)
#define PIN_C_D_LOW 		(GPIOC_PCOR|= 1<<PIN_C_D_LOC)
#define PIN_C_D_TOGGLE 		(GPIOC_PTOR|= 1<<PIN_C_D_LOC)

#define PIN_RST_LOC 4
#define PIN_RST_HIGH  		(GPIOC_PSOR|= 1<<PIN_RST_LOC)
#define PIN_RST_LOW 		(GPIOC_PCOR|= 1<<PIN_RST_LOC)
#define PIN_RST_TOGGLE 		(GPIOC_PTOR|= 1<<PIN_RST_LOC)


#define PIN_SCK_LOC 1
#define PIN_SCK_HIGH  		(GPIOD_PSOR|= 1<<PIN_SCK_LOC)
#define PIN_SCK_LOW 		(GPIOD_PCOR|= 1<<PIN_SCK_LOC)
#define PIN_SCK_TOGGLE 		(GPIOD_PTOR|= 1<<PIN_SCK_LOC)

#define PIN_MOSI_LOC 2
#define PIN_MOSI_HIGH  		(GPIOD_PSOR|= 1<<PIN_MOSI_LOC)
#define PIN_MOSI_LOW 		(GPIOD_PCOR|= 1<<PIN_MOSI_LOC)
#define PIN_MOSI_TOGGLE 	(GPIOD_PTOR|= 1<<PIN_MOSI_LOC)


#define PIN_CS_LOC 0
#define PIN_CS_HIGH  		(GPIOC_PSOR|= 1<<PIN_CS_LOC)
#define PIN_CS_LOW 			(GPIOC_PCOR|= 1<<PIN_CS_LOC)
#define PIN_CS_TOGGLE 		(GPIOC_PTOR|= 1<<PIN_CS_LOC)


void spi_bb_init(void);
void testIO(void);


#endif /* SOURCES_SPI_DRIVER_H_ */
