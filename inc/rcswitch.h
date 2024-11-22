/*
 *  Ported on: 02.18.2023
 *      Author: Jonathan Armstrong
 */

#ifndef RC_SWITCH_H
#define RC_SWITCH_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------
#include <stdbool.h>
#include <stdint.h>
//#include <stdio.h>

// original sui77/rc-switch stated:
// we can handle up to (unsigned long) => 32 bit * 2 H/L changes per bit + 2 for sync
#define RCSWITCH_MAX_CHANGES 67

// see comment below - I do not think these would ever be changed while the system is running
// so just define as constants here to save on memory space
// percent
#define N_RECEIVE_TOLERANCE   60
// microseconds
#define N_SEPARATION_LIMIT  4300

// number of repeating by default
//#define RF_TRANSMIT_REPEATS 8


// there is not a need to use a structure on a little 8051 microcontroller
//struct RC_SWITCH_T
//{
    // moved these to individual variables
//};


/**
 * Description of a single pulse, which consists of a high signal
 * whose duration is "high" times the base pulse length, followed
 * by a low signal lasting "low" times the base pulse length.
 * Thus, the pulse overall lasts (high+low)*pulseLength
 */
struct HighLow
{
    uint8_t high;
    uint8_t low;
};

/**
 * A "protocol" describes how zero and one bits are encoded into high/low
 * pulses.
 */
struct Protocol
{
    /** base pulse length in microseconds, e.g. 350 */
    uint16_t pulseLength;

    struct HighLow syncFactor;
    struct HighLow zero;
    struct HighLow one;
    
    /**
     * If true, interchange high and low logic levels in all transmissions.
     *
     * By default, RCSwitch assumes that any signals it sends or receives
     * can be broken down into pulses which start with a high signal level,
     * followed by a a low signal level. This is e.g. the case for the
     * popular PT 2260 encoder chip, and thus many switches out there.
     *
     * But some devices do it the other way around, and start with a low
     * signal level, followed by a high signal level, e.g. the HT6P20B. To
     * accommodate this, one can set invertedSignal to true, which causes
     * RCSwitch to change how it interprets any HighLow struct FOO: It will
     * then assume transmissions start with a low signal lasting
     * FOO.high*pulseLength microseconds, followed by a high signal lasting
     * FOO.low*pulseLength microseconds.
     */
    bool invertedSignal;
};

struct Pulse
{
	// we precompute these because doing the multiplies and divides
	// inside the transmit function interferes with generating proper signal timing
	uint16_t syncHigh;
	uint16_t syncLow;
	uint16_t zeroHigh;
	uint16_t zeroLow;
	uint16_t oneHigh;
	uint16_t oneLow;
	
	bool invertedSignal;
};



extern bool available(void);
extern void reset_available(void);

extern unsigned long get_received_value(void);
extern uint8_t       get_received_bitlength(void);
extern uint16_t      get_received_delay(void);
extern uint8_t       get_received_protocol(void);
extern int           get_received_tolerance(void);


//void setProtocol(struct Protocol protocol);
//void setProtocol(int nProtocol);
//void setProtocol(int nProtocol, int nPulseLength);

extern bool receive_protocol(const int p, unsigned int changeCount);
extern bool radio_tx_blocking(const uint8_t totalRepeats, const int protocolID);
extern void capture_handler(const uint16_t);

void setRepeatTransmit(const int repeat);
void setProtocol(const struct Protocol pro);

void send(struct Pulse* pro, unsigned char* packetPtr, const unsigned char bitsInPacket);


extern volatile __xdata uint16_t timings[RCSWITCH_MAX_CHANGES];


extern const struct Protocol protocols[];
extern const unsigned int numProto;

#endif // RC_SWITCH_H
