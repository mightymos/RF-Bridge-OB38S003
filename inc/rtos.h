#ifndef INC_RTOS_H_
#define INC_RTOS_H_

// function pointer
typedef void (*_fn)();

extern __idata unsigned char* gStackStart;

// this is the number of functions on the stack
#define MAIN_LEVEL      1
#define RTOS_RUN_LEVEL  1

// this is effectively two times a level because an address is formed from two bytes
#define MAIN_LENGTH         2
#define RTOS_RUN_LENGTH     2
//#define OS_SAVE_LENGTH      2
#define DELAY_LENGTH        2
#define OFF_BY_ONE_LENGTH   1


extern signed int create_process(_fn fn, int priority);
extern void idle();
extern void flash_led();
extern void rtos_init();
extern void rtos_run();
extern void send_radio_packet_task();

#endif // INC_RTOS_H_