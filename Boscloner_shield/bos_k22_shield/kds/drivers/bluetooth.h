/*
 * bluetooth.h
 *
 *  Created on: Feb 1, 2016
 *      Author: MJ
 */

#ifndef DRIVERS_BLUETOOTH_H_
#define DRIVERS_BLUETOOTH_H_
#include "boscloner_app.h"

//bluetooth enable =pta4
	//state = pta2

#define BT_EN_LOC  		5
#define BT_STATE_LOC 	 2

#define BT_EN()  		GPIOA_PSOR |= 1<<BT_EN_LOC
#define BT_DIS()  		GPIOA_PCOR |= 1<<BT_EN_LOC

void init_bt();
#endif /* DRIVERS_BLUETOOTH_H_ */
