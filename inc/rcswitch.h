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

// FIXME: how much can we save ?
#define RCSWITCH_MAX_CHANGES 64

// bucket sniffing constants
//#define MIN_FOOTER_LENGTH	300
//#define MIN_BUCKET_LENGTH	100

//#define PT226x_SYNC_MIN 4500

// sync constants
//#define TOLERANCE_MAX 500
//#define TOLERANCE_MIN 200

//
//#define TOTAL_RF_DATA_BITS 24

// number of repeating by default
//#define RF_TRANSMIT_REPEATS 8


// FIXME: some of these probably need to be changed to uint8_t or uint16_t as needed
struct RC_SWITCH_T
{
    // FIXME: we changed some ints to char so that printing over software uart was reasonable
    // would we ever have more than 256 protocols or bit length anyway ?
    unsigned long      nReceivedValue;
    unsigned char      nReceivedBitlength;
    unsigned int       nReceivedDelay;
    unsigned char      nReceivedProtocol;
    int                nReceiveTolerance;
    const unsigned int nSeparationLimit;
};


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
extern unsigned char get_received_bitlength(void);
extern unsigned int  get_received_delay(void);
extern unsigned char get_received_protocol(void);
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


extern volatile __xdata struct RC_SWITCH_T gRCSwitch;
extern volatile __xdata uint16_t timings[RCSWITCH_MAX_CHANGES];

//extern __xdata long long gTXRFData;

extern const struct Protocol protocols[];
extern const unsigned int numProto;

#endif // RC_SWITCH_H
