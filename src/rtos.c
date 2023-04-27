// RTOS framework port to 8051
// Author: Jonathan Armstrong

// Originally: Jason Losh, Brian Hendryx, Ganesh Krishnamurthy

// ----------------------------------------------------------------------------
//  Hardware connections
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//  Compiler directives and device includes
// --------------------------------------------------------------------------
#include "..\inc\delay.h"
#include "..\inc\initdevice.h"
#include "..\inc\globals.h"
#include "..\inc\rtos.h"

#include <8051.h>
#include <stdbool.h>

// ----------------------------------------------------------------------------
//  Pins and I/O Mapping
// ----------------------------------------------------------------------------


// ------------------------------------------------------------------------------
//  Defines
// ------------------------------------------------------------------------------

#define MAX_STACK_LEVELS 3

// typically need to also account for two more bytes attributable to storing rtos_run() address
#define START_OF_STACK 0x7c

// ------------------------------------------------------------------------------

// system timer tick
int timer_index;

// semaphore
#define MAX_QUEUE_SIZE 2
struct semaphore
{
  signed int count;
  int queue_size;
  int process_queue[MAX_QUEUE_SIZE]; // store task index here
};

__xdata struct semaphore *s;

// rtos status
int rtos_on = false;

// for uart interrupt
//volatile unsigned int rxChar = 0;

// FIXME: use enum instead?
// task 
#define STATE_INVALID    0 // no task
#define STATE_READY      1 // ready to run
#define STATE_BLOCKED    2 // has run, but now blocked by semaphore
#define STATE_DELAYED    3 // has run, but now awaiting timer

#define MAX_TASKS 2       // maximum number of valid tasks
int task_current = 0;      // index of last dispatched task
int task_count = 0;        // total number of valid tasks

// REQUIRED: add ability to store stack
struct _tcb
{
    // see STATE_ values above
    uint8_t state;
    // used to uniquely identify process
    uint16_t pid;
    // address in process to call next
    uint16_t addr[MAX_STACK_LEVELS];
    // saves number of stored stack levels
    uint8_t stack_count;
    // 0=lowest, 255=highest
    uint8_t priority;
    // tick when delay complete
    unsigned long tick;
    // used for prioritization
    uint8_t position;
};

// task control block
__xdata struct _tcb *tcb_ptr;
__xdata struct _tcb tcb[MAX_TASKS];

// ------------------------------------------------------------------------------
//  Prototypes      
// ------------------------------------------------------------------------------
int rtos_scheduler();

// debug helper
inline void stack_print(void)
{
    __idata unsigned char *stackPtr;
    
    for (stackPtr = START_OF_STACK; stackPtr <= SP; stackPtr++)
    {
        printf("stackPtr[%p]: 0x%x\r\n", stackPtr, *stackPtr);
    }
    
}

// debug helper
inline void tcb_print(void)
{
    unsigned char index;
    
    for (index = 0; index < tcb_ptr->stack_count; index++)
    {
        printf("tcb_ptr->addr[%u]: 0x%x\r\n", index, tcb_ptr->addr[index]);
    }
}

// specify inline so we do not place this function call on the stack while we mess with the stack
inline void os_context_save()
{
    __idata unsigned char *stackPtr;
    unsigned char index;
    
    unsigned char upperByte;
    unsigned char lowerByte;
    
    // DEBUG:
    printf("os_context_save()\r\n");
    printf("SP: 0x%02x\r\n", SP);
    
    // set pointer to top of stack
    stackPtr = (__idata unsigned char*) SP;
    
    // FIXME: fix comment
	// save number of levels to POPed off later EXCEPT rtos_run() at lower level AND yield(),wait(),os_context_save(), etc.
	tcb_ptr->stack_count = ((unsigned char)stackPtr - START_OF_STACK + 1 - 2) / 2;
    
    // DEBUG:
    printf("tcb_ptr->stack_count: %u\r\n", tcb_ptr->stack_count);
    
    // 
    index = tcb_ptr->stack_count;
    
    // skip over rtos_run() address
    stackPtr = (__idata unsigned char*) (START_OF_STACK + 2);
    
    //
    for (index = 0; index < tcb_ptr->stack_count; index++)
    {
        //
        lowerByte = *stackPtr;
        upperByte = *(stackPtr + 1);
        tcb_ptr->addr[index] = (upperByte << 8) | lowerByte;
        
        // traverse stack
        stackPtr += 2;
    }
    
    // DEBUG:
    stack_print();
    tcb_print();
}

inline void os_context_restore()
{
    __idata unsigned char *stackPtr;
    unsigned char index;
    
    printf("os_context_restore()\r\n");
    
    //SP = START_OF_STACK;
    // set pointer to bottom of stack and skip over rtos_run()
    stackPtr = START_OF_STACK + 2;
    
    // loop until top of stack which was previously saved is reached
    for (index = 0; index < tcb[task_current].stack_count; index++)
    {
        // index should begin at start/bottom of stack
        *stackPtr = (tcb_ptr->addr[index] & 0xff);
        *(stackPtr + 1) = (tcb_ptr->addr[index] >> 8) & 0xff;
        
        //printf("stackPtr: %p\r\n", stackPtr);
        //printf("stackPtr[0]: 0x%02x\r\n",  *stackPtr);
        //printf("stackPtr[+1]: 0x%02x\r\n", *(stackPtr+1));
        
        // traverse stack from bottom to top - hence increment
        stackPtr += 2;
    }
    
    // DEBUG:
    stack_print();
    tcb_print();
}

void rtos_init()
{
    int i;
    
    // indicate rtos off
    rtos_on = false;
    
    // no tasks running
    task_count = 0;
    
    // clear out tcb records
    for (i = 0; i < MAX_TASKS; i++)
    {
        tcb[i].state = STATE_INVALID;
        tcb[i].pid = 0;
        tcb[i].stack_count = 0;
        tcb[i].position = 0;
    }

    // setup timer 3 for 1ms system timer
    // with fosc/4 = 10 MHz and prescale = 1:8, then count = 1250 
    // count from 64286 to 65536, so init timer at 0xCF2C
    //set_timer3(0xFB1E);
    //setup_timer_3(T3_INTERNAL | T3_DIV_BY_8);
    // setup uart for 19200N1
    //setup_uart(19200);

    //enable_interrupts(INT_TIMER3);

    //Gank: init External Interrupt Dynamic Process
    // turn on uart interrupt
    //enable_interrupts(INT_RDA);
    //Gank end
    //enable_global_interrupts();
}

void init(void* p, int count)
{
  s = p;
  s->count = count;  
  s->queue_size = 0;
}

// REQUIRED: modify this function to wait a semaphore 
// return if avail, else yield to scheduler
void wait(void* p)
{
	// temporary stack address storage to make code more readable
	//uint16_t stackAddress;

	disable_global_interrupts();

	s = p;

	// no resource available ?
	if (s->count <= 0) {

		tcb_ptr = &tcb[task_current];
											
        // save current task in queue
		s->process_queue[s->queue_size++] = task_current;			
		
		// task won't run again until semphore signaled
		tcb_ptr->state = STATE_BLOCKED;

        // FIXME: correct?
        //os_context_save();

	}else{
		// use up one resource
		s->count--;
	}

	enable_global_interrupts();	
}

// REQUIRED: modify this function to signal a semaphore is available
void signal(void* p)
{
	// temp queue array index
	int	 index;

	disable_global_interrupts();

	s = p;
	// 1 more resource available
	s->count++;
	// resource avail. > resource needed ?
	if (s->count == 1) {
		// does some task even need the resource ?
		if (s->queue_size > 0) {
	        // handling need for one task 
			s->queue_size--;
			// retrieve task id from queue
			index = s->process_queue[s->queue_size];
			// unblock this task
			tcb_ptr = &tcb[index];	
			tcb_ptr->state = STATE_READY;		
			s->count--;	// todo: take this out	
		}
	}

	enable_global_interrupts();
}

// REQUIRED: modify this function to signal a semaphore is available
void signal_int(void* p)
{
	// do NOT enable/disable interrupts here

	s = p;
	// 1 more resource available
	s->count++;
	// resource avail. > resource needed ?
	if (s->count == 1) {
		// does some task even need the resource ?
		if (s->queue_size > 0) {
	        // handling need for one task 
			s->queue_size--;
			// retrive task, then unblock this task	
			tcb[s->process_queue[s->queue_size]].state = STATE_READY;
            // FIXME: previous comment said 'take this out'?
			s->count--;	
		}
	}
}

// REQUIRED: modify this function to yield execution back to scheduler
// COMPLETED: JDA
void yield()
{
	disable_global_interrupts();

    // DEBUG:
    printf("yield()\r\n");
    printf("SP: 0x%02x\r\n", SP);
	
	tcb_ptr = &tcb[task_current];

    // save stack
    os_context_save();
    

	// FIXME: this is wrong for nested function
    // so need to calculate from top of stack instead
	SP = START_OF_STACK + 3;
    
    // DEBUG:
    //printf("SP: 0x%02x\r\n", SP);

	enable_global_interrupts();

	// return with resumed task
}

// REQUIRED: modify this function to support 1ms system timer
// execution yielded back to scheduler until time elapses
void delay(long ticks)
{
	disable_global_interrupts();

	tcb_ptr = &tcb[task_current];

	tcb_ptr->tick = ticks;
	tcb_ptr->state = STATE_DELAYED;

    os_context_save();

	// FIXME: correct?
	SP = START_OF_STACK + 3;

	enable_global_interrupts();
}

// REQUIRED: modify this function to support prioritization
int rtos_scheduler()
{
  static int ok;
  static int task = 0xFF;
  static int task_run = 0;
  ok = false;
  
  while (!ok)
  {
    task++;
    if (task >= MAX_TASKS)
      task = 0;

	tcb_ptr = &tcb[task];

	if (tcb_ptr->state == STATE_READY)
    {
		if (tcb_ptr->position <= tcb_ptr->priority)
        {
			ok = true;
			task_run = task;
		}
	}
    
	tcb_ptr->position++;
  }
  
  return task_run;
}

void rtos_run()
{
    static _fn fn;
    unsigned char index;

    rtos_on = true;

    __idata unsigned char *stackPtr = SP;
  
    // DEBUG:
    printf("rtos_run()\r\n");
    printf("SP: 0x%02x\r\n", SP);
    printf("SP[0]: 0x%02x\r\n", *(stackPtr-1));
    printf("SP[1]: 0x%02x\r\n", *stackPtr);

  
    while(rtos_on)
    {
        task_current = rtos_scheduler();
        
        os_context_restore();

        // call next instruction in process
        index = tcb[task_current].stack_count - 1;
        fn = tcb[task_current].addr[index];

        // DEBUG:
        printf("fn: %p\r\n", fn);
        
        (*fn)();

        // FIXME: fix comment grammer for understanding
        // we don't, we get a new task/restore inside of yield()
        // yield(), wait(), delay(), and perhaps signal() will return to here
    }
}

void rtos_stop()
{
    rtos_on = false;
}

signed int create_process(_fn fn, int priority)
{
  signed int status = -1;
  int i = 0;
  int found = false;
  
  //disable_global_interrupts();  
  
  // save starting address if room in task list
  if (task_count < MAX_TASKS)
  {
    // make sure fn not already in list (prevent reentrancy)
    while (!found && (i < MAX_TASKS))
    {
      found = (tcb[i++].pid == fn);
    }
    
    if (!found)
    {
        // find first available tcb record
        i = 0;
        while (tcb[i].state != STATE_INVALID)
        {
            i++;
        }
        
        // load tcb record
        tcb[i].state = STATE_READY;
        // FIXME: variable indirection
        tcb[i].pid = (uint16_t) fn;
        // 1st stack level is begin. of fn
        tcb[i].addr[0] = (uint16_t) fn;
        // on 1st run, always 1 stack level
        tcb[i].stack_count = 1;
        tcb[i].priority = priority;    
        // increment task count
        task_count++;
        status = task_count;
    }
  }
  
  //enable_global_interrupts();  
  
  return status;
}

// REQUIRED: add code
void destroy_process(_fn fn)
{
	// indicator fn. is in tcb
	int found = false;
	// array index
	int i = 0;
    
	disable_global_interrupts();
 
	// loop thru all tcb tasks
	while(!found && (i < MAX_TASKS))
	{
		// fn. to destroy found ?
		found = (tcb[i++].pid == fn);  
	}

	// must negate last i++
	i--;
	// did we find the task (abort if current task!) ?
	if (found && (i != task_current))
	{
	 	tcb[i].pid = 0;
	 	tcb[i].state = STATE_INVALID;
		tcb[i].position = 0;

		// MUST decrement task count
		task_count--;

		// visually confirm destroy process
	  	//signal(&flash_req);
	 }
    
	enable_global_interrupts();
}


// ------------------------------------------------------------------------------
//  Task functions
// ------------------------------------------------------------------------------
// make sure all local variables are static

void idle()
{
    while(true) 
    {
        // DEBUG:
        //printf("idle...\r\n");
        
        // blocking delay
        led_toggle();
        //delay1ms(500);
        delay(500);
        
        yield();
    }
}

//-------------------------------------------
//dynamic process task
//-------------------------------------------
void flash_led()
{
    while(true)
    {
        led_on();
        // non-block delay
        delay(1000);
        led_off();
        delay(1000);
    }
}

void send_radio_packet_task()
{
    static unsigned char index;
    //unsigned char byteToSend;
    
    
    //enable_radio_vdd();
    //delay(RADIO_STARTUP_TIME);
    delay(500);
    
    // many receivers require repeatedly sending identical transmissions to accept data
    //for (index = 0; index < TX_REPEAT_TRANSMISSIONS; index++)
    for (index = 0; index < 1; index++)
    {
        // rf sync pulse
        reset_pin_on();
        //tdata_on();
        // FIXME: use correct units of delay
        delay(35);
        
        reset_pin_off();
        //tdata_off();
    
        // this should be the only really long delay required
        // FIXME: needs to be programmable
        delay(10);
        delay(85);

        // send rf key with unique id and code
        //send(0x4a);
        //send(0xf1);
        //send(rfcode);
    }
    
    //disable_radio_vdd();
    
    // FIXME: we need to force ask low/high just in case correct?
}

// ----------------------------------------------------------------------------
//  Reset and Startup Handlers
// ----------------------------------------------------------------------------


// REQUIRED: add code to manage blocked processes awaiting delay() completion
void rtos_timer(void) __interrupt (1)
{
	static int task = 0;
    
    // one millisecond overflow
    TH0 = 0xc1;
    TL0 = 0x7f;

	// initialize 'task' everytime
	task = 0;
    
	// loop thru all possible tasks in TCB
	while (task < MAX_TASKS)
    {

		tcb_ptr = &tcb[task];

		// is task delayed ?
		if (tcb_ptr->state == STATE_DELAYED)
        {
			// yes, decrement ticks
			tcb_ptr->tick--;
            
			// is ticks 0 ?
			if (tcb_ptr->tick == 0)
            {
				// yes, then task is ready
				tcb_ptr->state = STATE_READY;
			}
		}

		// check next task
		task++;
	}
}