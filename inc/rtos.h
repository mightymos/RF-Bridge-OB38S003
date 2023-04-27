#ifndef INC_RTOS_H_
#define INC_RTOS_H_

// function pointer
typedef void (*_fn)();


extern signed int create_process(_fn fn, int priority);
extern void idle();
extern void flash_led();
extern void rtos_init();
extern void rtos_run();
extern void send_radio_packet_task();

#endif // INC_RTOS_H_