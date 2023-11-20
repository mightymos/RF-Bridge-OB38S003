/*
  RCSwitch - Arduino libary for remote control outlet switches
  Copyright (c) 2011 Suat Özgür.  All right reserved.

  Contributors:
  - Andre Koehler / info(at)tomate-online(dot)de
  - Gordeev Andrey Vladimirovich / gordeev(at)openpyro(dot)com
  - Skineffect / http://forum.ardumote.com/viewtopic.php?f=2&t=46
  - Dominik Fischer / dom_fischer(at)web(dot)de
  - Frank Oltmanns / <first name>.<last name>(at)gmail(dot)com
  - Max Horn / max(at)quendi(dot)de
  - Robert ter Vehn / <first name>.<last name>(at)gmail(dot)com

  Project home: https://github.com/sui77/rc-switch/

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef _RCSwitch_h
#define _RCSwitch_h

#include <stdbool.h>
#include <stdint.h>

// Number of maximum high/Low changes per packet.
// We can handle up to (unsigned long) => 32 bit * 2 H/L changes per bit + 2 for sync
// Для keeloq нужно увеличить RCSWITCH_MAX_CHANGES до 23+1+66*2+1=157
#define RCSWITCH_MAX_CHANGES 67        // default 67

// separationLimit: minimum microseconds between received codes, closer codes are ignored.
// according to discussion on issue #14 it might be more suitable to set the separation
// limit to the same time as the 'low' part of the sync signal for the current protocol.
// should be set to the minimum value of pulselength * the sync signal
#define RCSWITCH_SEPARATION_LIMIT 4100


bool available(void);
void reset_available(void);

unsigned long long get_received_value(void);
unsigned int get_received_bitlength(void);
unsigned int get_received_delay(void);
unsigned int get_received_protocol(void);
unsigned int* get_received_rawdata(void);
uint8_t get_num_protos(void);

extern bool receive_protocol(const int p, unsigned int changeCount);

/**
 * Description of a single pule, which consists of a high signal
 * whose duration is "high" times the base pulse length, followed
 * by a low signal lasting "low" times the base pulse length.
 * Thus, the pulse overall lasts (high+low)*pulseLength
 */
struct HighLow {
    uint8_t high;
    uint8_t low;
};

/**
 * A "protocol" describes how zero and one bits are encoded into high/low
 * pulses.
 */
struct Protocol {
    /** base pulse length in microseconds, e.g. 350 */
    uint16_t pulseLength;
    uint8_t PreambleFactor;
    struct HighLow Preamble;
    uint8_t HeaderFactor;
    struct HighLow Header;

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
    uint16_t Guard;
};





/*
 * timings[0] contains sync timing, followed by a number of bits
 */
extern volatile __xdata unsigned int timings[RCSWITCH_MAX_CHANGES];

// буфер длительностей последних четырех пакетов, [0] - последний
extern volatile __xdata unsigned int buftimings[4];

#endif
