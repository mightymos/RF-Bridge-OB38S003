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

// FIXME: force fixed address or set at startup somehow?
// typically need to also account for two more bytes attributable to storing rtos_run() address
//#define START_OF_STACK 0x81
__idata unsigned char* gStackStart;

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

// maximum number of valid tasks
#define MAX_TASKS 2
// index of last dispatched task
int gTaskCurrent = 0;
// total number of valid tasks
int gTaskCount = 0;

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
__xdata struct _tcb *gTcbPtr;
__xdata struct _tcb tcb[MAX_TASKS];

// ------------------------------------------------------------------------------
//  Prototypes      
// ------------------------------------------------------------------------------
int rtos_scheduler();


void stack_print(void)
{
    __idata unsigned char* stackPtr;
    
    printf("stack_print()\r\n");
    
    for (stackPtr = gStackStart; stackPtr <= (__idata unsigned char*) SP; stackPtr++)
    {
        printf("stackPtr[%p]: 0x%02x\r\n", stackPtr, *stackPtr);
    }
    
}

// debug helper
void task_stack_print(void)
{
    __idata unsigned char* stackPtr;
    unsigned char index;
    
    printf("task_stack_print()\r\n");
    
    //for (stackPtr = gStackStart; stackPtr <= (__idata unsigned char*) SP; stackPtr++)
    stackPtr = gStackStart;
    for (index = 0; index < gTcbPtr->stack_count + MAIN_LEVEL + RTOS_RUN_LEVEL; index++)
    {
        printf("stackPtr[%p]: 0x%02x\r\n", stackPtr, *stackPtr);
        printf("stackPtr[%p]: 0x%02x\r\n", stackPtr + 1, *(stackPtr + 1));
        
        stackPtr += 2;
    }
    
}

// debug helper
void tcb_print(void)
{
    unsigned char index;
    
    printf("tcb_print()\r\n");
    
    for (index = 0; index < gTcbPtr->stack_count; index++)
    {
        printf("gTcbPtr->addr[%u]: 0x%04x\r\n", index, gTcbPtr->addr[index]);
    }
}

void os_context_save()
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
	// save number of levels to be POPed off later EXCEPT rtos_run() at lower level AND yield(),wait(),os_context_save(), etc.
	//gTcbPtr->stack_count = ((unsigned char)stackPtr - (unsigned char) gStackStart + 1 - 4) / 2;
    gTcbPtr->stack_count = ((unsigned char)stackPtr - MAIN_LENGTH - RTOS_RUN_LENGTH - DELAY_LENGTH - (unsigned char) gStackStart + OFF_BY_ONE_LENGTH) / 2;
    
    // DEBUG:
    printf("gTcbPtr->stack_count: %u\r\n", gTcbPtr->stack_count);
    
    // 
    index = gTcbPtr->stack_count;
    
    // skip over rtos_run() address
    stackPtr = gStackStart + MAIN_LENGTH + RTOS_RUN_LENGTH;
    
    //
    for (index = 0; index < gTcbPtr->stack_count; index++)
    {
        //
        lowerByte = *stackPtr;
        upperByte = *(stackPtr + 1);
        gTcbPtr->addr[index] = (upperByte << 8) | lowerByte;
        
        // traverse stack
        stackPtr += 2;
    }
    
    // DEBUG:
    stack_print();
    tcb_print();
}

// specify inline so we do not place this function call on the stack while we mess with the stack
// FIXME: would like to remove the need to inline
void os_context_restore()
{
    __idata unsigned char* stackPtr;
    unsigned char index;
    
    unsigned char lowAddr;
    unsigned char highAddr;

    
    // DEBUG:
    printf("os_context_restore()\r\n");
    
    // save the return address for this function because restored stack might overwrite
    stackPtr = (__idata unsigned char*) SP;
    //lowAddr  = *(stackPtr - 1);
    //highAddr = *stackPtr;
    
    // DEBUG:
    printf("stackPtr[%p]: 0x%02x\r\n", stackPtr - 1, *(stackPtr - 1));
    printf("stackPtr[%p]: 0x%02x\r\n", stackPtr, *stackPtr);

    
    // set pointer to bottom of stack and skip over rtos_run()
    stackPtr = gStackStart + MAIN_LENGTH + RTOS_RUN_LENGTH;
    
    SP = (uint8_t) stackPtr;
    
    // DEBUG:
    printf("stack_count: %u\r\n", gTcbPtr->stack_count);
    
    // loop until top of stack which was previously saved is reached
    for (index = 0; index < gTcbPtr->stack_count; index++)
    {
        // index should begin at start/bottom of stack
        *stackPtr = gTcbPtr->addr[index] & 0xff;
        
        SP++;
        
        *(stackPtr + 1) = (gTcbPtr->addr[index] >> 8) & 0xff;
        
        SP++;
        
        // DEBUG:
        printf("gTcbPtr->addr[%u]: 0x%04x\r\n", index, gTcbPtr->addr[index]);
        printf("stackPtr[%p]: 0x%02x\r\n", stackPtr,   *stackPtr);
        printf("stackPtr[%p]: 0x%02x\r\n", stackPtr+1, *(stackPtr + 1));
        
        // traverse stack from bottom to top - hence increment
        stackPtr += 2;
    }
    
    // place return address for this function on top of restored stack
    //*stackPtr = lowAddr;
    //SP++;
    //*(stackPtr + 1) = highAddr;
    //SP++;
    
    // DEBUG:
    task_stack_print();
    stack_print();
    tcb_print();
    
    
    // DEBUG:
    stackPtr = (__idata unsigned char*) SP;
    printf("stackPtr[%p]: 0x%02x\r\n", stackPtr - 1, *(stackPtr - 1));
    printf("stackPtr[%p]: 0x%02x\r\n", stackPtr, *stackPtr);
    
    SP = (uint8_t) gStackStart + MAIN_LENGTH + RTOS_RUN_LENGTH;
}

void rtos_init()
{
    int i;
    
    // indicate rtos off
    rtos_on = false;
    
    // no tasks running
    gTaskCount = 0;
    
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

// yield execution back to scheduler
void yield()
{
	disable_global_interrupts();

    // DEBUG:
    printf("yield()\r\n");
    printf("SP: 0x%02x\r\n", SP);
	
	gTcbPtr = &tcb[gTaskCurrent];

    // save stack
    // FIXME: assume pointer has been set to current task?
    os_context_save();
    

	// FIXME: this is wrong for nested function
    // so need to calculate from top of stack instead
	SP = (uint8_t) gStackStart + MAIN_LENGTH + RTOS_RUN_LENGTH;

	enable_global_interrupts();
    
    // return to rtos_run()
}

// function uses 1ms system timer
// execution yielded back to scheduler until time elapses
void delay(long ticks)
{
	disable_global_interrupts();
    
    // DEBUG:
    printf("delay()\r\n");
    printf("SP: 0x%02x\r\n", SP);

	gTcbPtr = &tcb[gTaskCurrent];

	gTcbPtr->tick = ticks;
	gTcbPtr->state = STATE_DELAYED;

    os_context_save();

	// FIXME: correct?
	SP = (uint8_t) gStackStart + MAIN_LENGTH + RTOS_RUN_LENGTH;

	enable_global_interrupts();
}

// FIXME: modify this function to support prioritization - completed?
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
        {
            task = 0;
        }

        gTcbPtr = &tcb[task];

        if (gTcbPtr->state == STATE_READY)
        {
            if (gTcbPtr->position <= gTcbPtr->priority)
            {
                ok = true;
                task_run = task;
            }
        }

        gTcbPtr->position++;
    }

    return task_run;
}

// yielded/delayed/etc tasks will return here and then next task gets identified and run
void rtos_run()
{
    static _fn fn;
    unsigned char index;

    rtos_on = true;

    __idata unsigned char* stackPtr = (__idata unsigned char*) SP;
  
    // DEBUG:
    printf("rtos_run()\r\n");
    printf("SP: 0x%02x\r\n", SP);
    printf("SP[0]: 0x%02x\r\n", *(stackPtr-1));
    printf("SP[1]: 0x%02x\r\n", *stackPtr);

  
    while(rtos_on)
    {
        // get next task
        gTaskCurrent = rtos_scheduler();
        
        // current task should just be zero on first entry        
        // DEBUG:
        printf("gTaskCurrent: %u\r\n", gTaskCurrent);

        // assign pointer to be used as shorthand in context restore
        gTcbPtr = &tcb[gTaskCurrent];
        
        // assumes tcb pointer has been set to current task
        os_context_restore();
        
        // FIXME: think that SP needs to be set here

        // call next instruction in process
        index = tcb[gTaskCurrent].stack_count - 1;
        fn = (_fn) tcb[gTaskCurrent].addr[index];

        // DEBUG:
        printf("fn: %p\r\n", fn);
        
        (*fn)();

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
    int index = 0;
    bool found = false;
    const bool flag = global_interrupts_are_enabled();

    // FIXME: necessary?
    // checking if globals are enabled in the first place
    // so that we do not prematurely enable them at the end of create_process
    // however, we might have to disable/re-enable if creating processes dynamically
    // (i.e., while rtos is running)
    if (flag)
    {
        disable_global_interrupts();  
    }
  
    // save starting address if room in task list
    if (gTaskCount < MAX_TASKS)
    {
        // make sure fn not already in list (prevent reentrancy)
        while (!found && (index < MAX_TASKS))
        {
          found = (tcb[index++].pid == (uint16_t) fn);
        }

        if (!found)
        {
            // find first available tcb record
            index = 0;
            while (tcb[index].state != STATE_INVALID)
            {
                index++;
            }
            
            // load tcb record
            tcb[index].state = STATE_READY;
            // we store what are really function pointers as unsigned int
            // because we need to manipulate the addresses from 8-bit values on the stack
            tcb[index].pid = (uint16_t) fn;
            // 1st stack level is begin. of fn
            tcb[index].addr[0] = (uint16_t) fn;
            // on 1st run, always 1 stack level
            tcb[index].stack_count = 1;
            tcb[index].priority = priority; 
            
            // increment task count
            gTaskCount++;
            status = gTaskCount;
        }
    }

    if (flag)
    {
        enable_global_interrupts();
    }

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
		found = (tcb[i++].pid == (uint16_t) fn);  
	}

	// must negate last i++
	i--;
	// did we find the task (abort if current task!) ?
	if (found && (i != gTaskCurrent))
	{
	 	tcb[i].pid = 0;
	 	tcb[i].state = STATE_INVALID;
		tcb[i].position = 0;

		// MUST decrement task count
		gTaskCount--;

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
        printf("idle()\r\n");
        
        //led_toggle();
        
        // blocking delay
        delay1ms(1);
        
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
        // DEBUG:
        printf("flash_led()\r\n");
        
        led_toggle();
        
        // non-blocking delay
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

		gTcbPtr = &tcb[task];

		// is task delayed ?
		if (gTcbPtr->state == STATE_DELAYED)
        {
			// yes, decrement ticks
			gTcbPtr->tick--;
            
			// is ticks 0 ?
			if (gTcbPtr->tick == 0)
            {
				// yes, then task is ready
				gTcbPtr->state = STATE_READY;
			}
		}

		// check next task
		task++;
	}
}