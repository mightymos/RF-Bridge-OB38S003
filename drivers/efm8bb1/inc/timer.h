#ifndef INC_TIMERS_H_
#define INC_TIMERS_H_


unsigned long get_time_milliseconds(void);
unsigned long get_time_ten_microseconds(void);

void InitTimer2_us(uint16_t interval, uint16_t timeout);
void InitTimer3_us(uint16_t interval, uint16_t timeout);
void InitTimer2_ms(uint16_t interval, uint16_t timeout);
void InitTimer3_ms(uint16_t interval, uint16_t timeout);
void WaitTimer2Finished(void);
void WaitTimer3Finished(void);
void StopTimer2(void);
void StopTimer3(void);
bool IsTimer2Finished(void);
bool IsTimer3Finished(void);

#endif