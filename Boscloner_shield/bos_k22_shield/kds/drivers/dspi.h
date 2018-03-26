/*
 * dspi.h
 *
 *  Created on: Jan 22, 2016
 *      Author: MJ
 */

#ifndef APP_DSPI_H_
#define APP_DSPI_H_
#include "stdint.h"

extern void SPI1_IRQHandler(void);
int8_t test_dspi(void);
void send_clone_cmd_pm3();

#endif /* APP_DSPI_H_ */
