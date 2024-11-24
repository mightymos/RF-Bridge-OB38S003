#include "portisch.h"
#include "portisch_command_format.h"
#include "portisch_protocols.h"
#include "portisch_serial.h"
#include "uart.h"


void uart_put_RF_Data_Standard(uint8_t command)
{
	uint8_t index = 0;

	uart_putc(RF_CODE_START);
	uart_putc(command);

	// sync low time
	uart_putc((SYNC_LOW >> 8) & 0xFF);
	uart_putc(SYNC_LOW & 0xFF);
	// bit 0 high time
	uart_putc((BIT_LOW >> 8) & 0xFF);
	uart_putc(BIT_LOW & 0xFF);
	// bit 1 high time
	uart_putc((BIT_HIGH >> 8) & 0xFF);
	uart_putc(BIT_HIGH & 0xFF);

	// copy data to UART buffer
	index = 0;

	// FIXME: used to say 24/8 but in any case would be better to avoid magic numbers
	while(index < 3)
	{
		uart_putc(RF_DATA[index]);
		index++;
	}
    
	uart_putc(RF_CODE_STOP);
}


void uart_put_RF_Data_Advanced(uint8_t command, uint8_t protocol_index)
{
	uint8_t index = 0;
	uint8_t b = 0;
	uint8_t bits = 0;

	uart_putc(RF_CODE_START);
	uart_putc(command);

	bits = PROTOCOL_DATA[protocol_index].bit_count;

	while(index < bits)
	{
		index += 8;
		b++;
	}

	uart_putc(b+1);

	// send index off this protocol
	uart_putc(protocol_index);

	// copy data to UART buffer
	index = 0;
	while(index < b)
	{
		uart_putc(RF_DATA[index]);
		index++;
	}
    
	uart_putc(RF_CODE_STOP);

}



// for bucket sniffing
// a state machine might be ideal for this eventually
// but unclear if it is really necessary
void uart_put_RF_buckets(uint8_t command)
{
	uint8_t index = 0;

	uart_putc(RF_CODE_START);
	uart_putc(command);
    
	// put bucket count + sync bucket
	uart_putc(bucket_count + 1);

	// because the sniffing command (0xB1) can transmit a significant amount of bytes
    // and we do not have so much ram to allocate as a buffer
    // we normally check for buffer bytes to transmit by uart in the main loop
    // but there is potentially so much data here that we would overflow prior to returning to main loop
    // so just send in pieces instead

    while(!is_uart_tx_buffer_empty())
    {
        if (is_uart_tx_finished())
        {
            // if not empty, set TI, which triggers interrupt to actually transmit
            uart_init_tx_polling();
        }
    }

	// send up to 7 buckets
	while (index < bucket_count)
	{
		uart_putc((buckets[index] >> 8) & 0x7F);
		uart_putc(buckets[index] & 0xFF);
		index++;
	}

	// send sync bucket
	uart_putc((bucket_sync >> 8) & 0x7F);
	uart_putc(bucket_sync & 0xFF);

	// start and wait for transmit
    while(!is_uart_tx_buffer_empty())
    {
        if (is_uart_tx_finished())
        {
            uart_init_tx_polling();
        }
    }

	index = 0;
    
	while(index < actual_byte)
	{
		uart_putc(RF_DATA[index]);
		index++;

		// be safe so as to avoid buffer overflow
        // FIXME: could modulo divide be worked into ring buffer logic itself?
		if ((index % UART_TX_BUFFER_SIZE) == 0)
		{
			// start and wait for transmit
            while(!is_uart_tx_buffer_empty())
            {
                if (is_uart_tx_finished())
                {
                    uart_init_tx_polling();
                }
            }
		}
	}

	uart_putc(RF_CODE_STOP);

}