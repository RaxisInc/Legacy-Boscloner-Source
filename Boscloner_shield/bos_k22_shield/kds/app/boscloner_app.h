/*
 * boscloner.h
 *
 *  Created on: Jan 23, 2016
 *      Author: MJ
 */

#ifndef APP_BOSCLONER_APP_H_
#define APP_BOSCLONER_APP_H_

//global app defines
#include <stdint.h>
#include "ksdk.h"
#include "spi_bb_driver.h"	//bit bang master driver
#include "dspi.h"		//spi slave driver
#include "oled_app.h"
#include "Adafruit_GFX.h"
#include "printf.h"
#include "oled_app.h"
#include "uart_bm.h"
#include "bluetooth.h"
#include "Timers.h"
#include "bos_io.h"
#include "weigand.h"


uint8_t dsp_tmr1 = 0;
#define DEF_DSP_UPDATE_TIME 2000	//basic updates of system status
#define CARD_ID_UPDATE_TIME 10000	//10 seconds

#define BOS_PACKET_SIZE 16

#define BOSCLONER_VERSION "1.0"


typedef enum bos_state_{
	IDLE,	//waiting
	TX,		//tx data to pm3
	RX		//rx data from pm3
}bos_state_t;

extern bos_state_t bos_state;



#endif /* APP_BOSCLONER_APP_H_ */


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
