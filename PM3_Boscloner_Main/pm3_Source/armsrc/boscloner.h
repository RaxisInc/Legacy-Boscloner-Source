#ifndef __BOSCLONER_H
#define __BOSCLONER_H
#include "stdint.h"

#define BOS_LOOP_CNT_VAL 200000  //aprox 1000ms
//#define BOS_LOOP_CNT_VAL 20000  //aprox 100ms
//#define BOS_LOOP_CNT_VAL 500  //aprox
//#define BOS_LOOP_CNT_VAL 50
//aprox

extern uint8_t bos_clone_id[];
extern uint8_t bos_clone_status;


extern uint32_t copy_count;
extern uint8_t copy_run;
extern uint8_t clone_run;
extern uint8_t flag_clone;

typedef enum bos_clone_state_ {
	WAIT, CLONE, DONE,
} bos_clone_state_t;
extern bos_clone_state_t bos_clone_state;

void boscloner_setupspi(void);
void boscloner_send_spi();
void boslconer_heartbeat(void);
void boscloner_init(void);
void boscloner_tasks(void);
uint32_t get_id_hi();
uint32_t get_id_lo();



#endif
