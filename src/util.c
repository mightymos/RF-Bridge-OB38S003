#include "uart.h"

void putstring(const char *s)
{
   while (*s) uart_putc(*s++);
}

void puthex(unsigned char v)
{
   unsigned char c;
   v &= 0x0f;
   if (v<10) c = '0'+v;
   else c = 'A'-10+v;
   uart_putc(c);
}

void puthex2(const unsigned char x)
{
   puthex(x >> 4);
   puthex(x);
}