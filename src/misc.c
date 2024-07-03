/*! \brief Check if XRAM was properly initialized.
 *         Uses up four bytes of XRAM.
 *
 * 'XRAMArray' should be initialized to "DEADBEEF" if XRAM
 *  was enabled properly (by sdcc_external_startup()).
 *  Function performs a sanity check to be sure that this is the case.
 */
int8_t xram_test(void)
{   
    /* be sure this variable is defined with __xdata keyword
       - if the following routine fails start by suspecting hardware connections if appropriate
          (ex. for external memory) and sdcc_external_startup() routine first
    */
    __xdata uint8_t XRAMArray[] = {0xDE,0xAD,0xBE,0xEF};
    
    /* stored in read only code ROM -> string "DEADBEEF" */
    const uint8_t  ROMArray[] = {0xDE,0xAD,0xBE,0xEF};
    
    int8_t result = 0;
    uint8_t length;
    uint8_t i;
    
    /* obtain number of elements in array stored in ROM */
    length = sizeof(ROMArray) / sizeof(uint8_t);
    
    /* loop thru all array positions */
    for (i = 0; i < length; i++)
    {
        /* check if __xdata variable matches expected initialization value mirrored in ROM */
        if (XRAMArray[i] != ROMArray[i])
        {
            result = -1;
        }
    }
    
    /* NOTE: could indicate error with an assert() too possibly versus return value... */
    
    return result;
}



/*! \brief Check if XRAM is working across all address lines
 *  NOTE: it is important to make some variables used here 'volatile'
 *   so that the optimizing compiler does not modify logic depending
 *   on these variables
 */
 int8_t extended_xram_test(void)
{   
    /* size of external sram chip */
    /* note: magic number is max. size of our external SRAM (32 KB) */
    const unsigned int length = 256;
    //const unsigned int length = 65535;
    
    /* pointer to external memory mapped space */
    volatile __xdata unsigned char * __data ptr;
    
    /* value to write to external sram memory */
    volatile unsigned char testValue;
    
    /* index variable */
    unsigned int i;
    
    /* track read/write errors */
    unsigned int errorCount;
    
    /* display test info. */
    //printf_fast("extended_xram_test()\r\n");
    
    /* initialize memory pointer - assumes we can start at 0 address */
    ptr = 0x0;
    testValue = 0x0;
    errorCount = 0;
    
    /* display the amount of memory to be considered */
    printf_fast("xram: %u\r\n", length);
    
    /* write memory and then read it back */
    for (i = 0; i < length; i++)
    {
        /* write memory location - using array notation here */
        ptr[i] = testValue;
        
        
        // error in reading back value?
        if (ptr[i] != testValue)
        {
            /* do not allow variable to overflow */
            if (errorCount != length)
            {
                errorCount++;
            }
        }
        
        /* this will overflow every 256 increments on our processor */
        testValue++;
    }
    
    /* display number of errors encountered (if any) */
    printf_fast("errors: %u\r\n", errorCount);
    
    return 0;
}
