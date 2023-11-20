/*
  RCSwitch - Arduino libary for remote control outlet switches
  Copyright (c) 2011 Suat Özgür.  All right reserved.

  Contributors:
  - Andre Koehler / info(at)tomate-online(dot)de
  - Gordeev Andrey Vladimirovich / gordeev(at)openpyro(dot)com
  - Skineffect / http://forum.ardumote.com/viewtopic.php?f=2&t=46
  - Dominik Fischer / dom_fischer(at)web(dot)de
  - Frank Oltmanns / <first name>.<last name>(at)gmail(dot)com
  - Andreas Steinel / A.<lastname>(at)gmail(dot)com
  - Max Horn / max(at)quendi(dot)de
  - Robert ter Vehn / <first name>.<last name>(at)gmail(dot)com
  - Johann Richard / <first name>.<last name>(at)gmail(dot)com
  - Vlad Gheorghe / <first name>.<last name>(at)gmail(dot)com https://github.com/vgheo

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

#include <stdlib.h>
#include <string.h>

#include "rcswitch_tasmota.h"

/*
 * timings[0] contains sync timing, followed by a number of bits
 */
volatile __xdata unsigned int timings[RCSWITCH_MAX_CHANGES];

// буфер длительностей последних четырех пакетов, [0] - последний
volatile __xdata unsigned int buftimings[4];


__xdata int nReceiveTolerance = 60;
volatile __xdata unsigned long long nReceivedValue;
volatile __xdata unsigned long long nReceiveProtocolMask = (1ULL << numProto) - 1;
volatile __xdata unsigned int nReceivedBitlength;
volatile __xdata unsigned int nReceivedDelay;
volatile __xdata unsigned int nReceivedProtocol;
const __xdata unsigned int nSeparationLimit = RCSWITCH_SEPARATION_LIMIT;


/* Protocol description format
 *
 * {
 *    Pulse length,
 *
 *    PreambleFactor,
 *    Preamble {high,low},
 *
 *    HeaderFactor,
 *    Header {high,low},
 *
 *    "0" bit {high,low},
 *    "1" bit {high,low},
 *
 *    Inverted Signal,
 *    Guard time
 * }
 *
 * Pulse length: pulse duration (Te) in microseconds,
 *               for example 350
 * PreambleFactor: Number of high and low states to send
 *                 (One pulse = 2 states, in orther words, number of pulses is
 *                 ceil(PreambleFactor/2).)
 * Preamble: Pulse shape which defines a preamble bit.
 *           Sent ceil(PreambleFactor/2) times.
 *           For example, {1, 2} with factor 3 would send
 *      _    _
 *     | |__| |__         (each horizontal bar has a duration of Te,
 *                         vertical bars are ignored)
 * HeaderFactor: Number of times to send the header pulse.
 * Header: Pulse shape which defines a header (or "sync"/"clock") pulse.
 *           {1, 31} means one pulse of duration 1 Te high and 31 Te low
 *      _
 *     | |_______________________________ (don't count the vertical bars)
 *
 * "0" bit: pulse shape defining a data bit, which is a logical "0"
 *          {1, 3} means 1 pulse duration Te high level and 3 low
 *      _
 *     | |___
 *
 * "1" bit: pulse shape that defines the data bit, which is a logical "1"
 *          {3, 1} means 3 pulses with a duration of Te high level and 1 low
 *      ___
 *     |   |_
 *
 * (note: to form the state bit Z (Tri-State bit), two codes are combined)
 *
 * Inverted Signal: Signal inversion - if true the signal is inverted
 *                  replacing high to low in a transmitted / received packet
 * Guard time: Separation time between two retries. It will be followed by the
 *             next preamble of the next packet. In number of Te.
 *             e.g. 39 pulses of duration Te low level
 */

const struct Protocol protocols[] = {
  { 350,  0, { 0, 0 }, 1, {  1, 31 }, { 1,  3 }, { 3, 1 }, false,  0 },  // 01 (Princeton, PT-2240)
  { 650,  0, { 0, 0 }, 1, {  1, 10 }, { 1,  2 }, { 2, 1 }, false,  0 },  // 02
  { 100,  0, { 0, 0 }, 1, { 30, 71 }, { 4, 11 }, { 9, 6 }, false,  0 },  // 03
  { 380,  0, { 0, 0 }, 1, {  1,  6 }, { 1,  3 }, { 3, 1 }, false,  0 },  // 04
  { 500,  0, { 0, 0 }, 1, {  6, 14 }, { 1,  2 }, { 2, 1 }, false,  0 },  // 05
  { 450,  0, { 0, 0 }, 1, { 23,  1 }, { 1,  2 }, { 2, 1 }, true,   0 },  // 06 (HT6P20B)
  { 150,  0, { 0, 0 }, 1, {  2, 62 }, { 1,  6 }, { 6, 1 }, false,  0 },  // 07 (HS2303-PT, i. e. used in AUKEY Remote)
  { 320,  0, { 0, 0 }, 1, { 36,  1 }, { 1,  2 }, { 2, 1 }, true,   0 },  // 08 (Came 12bit, HT12E)
  { 700,  0, { 0, 0 }, 1, { 32,  1 }, { 1,  2 }, { 2, 1 }, true,   0 },  // 09 (Nice_Flo 12bit)
  { 420,  0, { 0, 0 }, 1, { 60,  6 }, { 1,  2 }, { 2, 1 }, true,   0 },  // 10 (V2 phoenix)
  { 500,  2, { 3, 3 }, 0, {  0,  0 }, { 1,  2 }, { 2, 1 }, false, 37 },  // 11 (Nice_FloR-S 52bit)
  { 400, 23, { 1, 1 }, 1, {  0,  9 }, { 2,  1 }, { 1, 2 }, false, 39 },  // 12 Placeholder not working! (Keeloq 64/66)
  { 300,  6, { 2, 2 }, 3, {  8,  3 }, { 2,  2 }, { 3, 3 }, false,  0 },  // 13 test (CFM)
  { 250, 12, { 4, 4 }, 0, {  0,  0 }, { 1,  1 }, { 2, 2 }, false,  0 },  // 14 test (StarLine)
  { 500,  0, { 0, 0 }, 0, { 100, 1 }, { 1,  2 }, { 2, 1 }, false, 35 },  // 15

  { 361,  0, { 0, 0 }, 1, {  52,  1 }, { 1,  3 }, { 3, 1 }, true,   0 }, // 16 (Einhell)
  { 500,  0, { 0, 0 }, 1, {   1, 23 }, { 1,  2 }, { 2, 1 }, false,  0 }, // 17 (InterTechno PAR-1000)
  { 180,  0, { 0, 0 }, 1, {   1, 15 }, { 1,  1 }, { 1, 8 }, false,  0 }, // 18 (Intertechno ITT-1500)
  { 350,  0, { 0, 0 }, 1, {   1,  2 }, { 0,  2 }, { 3, 2 }, false,  0 }, // 19 (Murcury)
  { 150,  0, { 0, 0 }, 1, {  34,  3 }, { 1,  3 }, { 3, 1 }, false,  0 }, // 20 (AC114)
  { 360,  0, { 0, 0 }, 1, {  13,  4 }, { 1,  2 }, { 2, 1 }, false,  0 }, // 21 (DC250)
  { 650,  0, { 0, 0 }, 1, {   1, 10 }, { 1,  2 }, { 2, 1 }, true,   0 }, // 22 (Mandolyn/Lidl TR-502MSV/RC-402/RC-402DX)
  { 641,  0, { 0, 0 }, 1, { 115,  1 }, { 1,  2 }, { 2, 1 }, true,   0 }, // 23 (Lidl TR-502MSV/RC-402 - Flavien)
  { 620,  0, { 0, 0 }, 1, {   0, 64 }, { 0,  1 }, { 1, 0 }, false,  0 }, // 24 (Lidl TR-502MSV/RC701)
  { 560,  0, { 0, 0 }, 1, {  16,  8 }, { 1,  1 }, { 1, 3 }, false,  0 }, // 25 (NEC)
  { 385,  0, { 0, 0 }, 1, {   1, 17 }, { 1,  2 }, { 2, 1 }, false,  0 }, // 26 (Arlec RC210)
  { 188,  0, { 0, 0 }, 1, {   1, 31 }, { 1,  3 }, { 3, 1 }, false,  0 }, // 27 (Zap, FHT-7901)

  { 700,  1, { 0, 1 }, 1, { 116,  0 }, { 1,  2 }, { 2, 1 }, true,   0 }, // 28 (Quigg GT-7000) from @Tho85 https://github.com/sui77/rc-switch/pull/115
  { 220,  0, { 0, 0 }, 1, {   1, 46 }, { 1,  6 }, { 1, 1 }, false,  2 }, // 29 (NEXA)
  { 260,  0, { 0, 0 }, 1, {   1,  8 }, { 1,  4 }, { 4, 1 }, true,   0 },  // 30 (Anima)

  { 400,  0, { 0, 0 }, 1, {   1,  1 }, { 1,  2 }, { 2, 1 }, false, 43 },  // 31 (Mertik Maxitrol G6R-H4T1)
  { 365,  0, { 0, 0 }, 1, {  18,  1 }, { 3,  1 }, { 1, 3 }, true,   0 },  // 32 (1ByOne Doorbell) from @Fatbeard https://github.com/sui77/rc-switch/pull/277
  { 340,  0, { 0, 0 }, 1, {  14,  4 }, { 1,  2 }, { 2, 1 }, false,  0 },  // 33 (Dooya Control DC2708L)
  { 120,  0, { 0, 0 }, 1, {   1, 28 }, { 1,  3 }, { 3, 1 }, false,  0 },  // 34 DIGOO SD10 - so as to use this protocol RCSWITCH_SEPARATION_LIMIT must be set to 2600
  { 20,   0, { 0, 0 }, 1, { 239, 78 }, {20, 35 }, {35, 20}, false, 10000},// 35 Dooya 5-Channel blinds remote DC1603
  { 250,  0, { 0, 0 }, 1, {  18,  6 }, { 1,  3 }, { 3, 1 }, false,  0 },   // 36 Dooya remote DC2700AC for Dooya DT82TV curtains motor
  { 200,  0, { 0, 0 }, 0, {   0,  0 }, { 1,  3 }, { 3, 1} , false, 20}	  // 37 DEWENWILS Power Strip
};

enum {
   numProto = sizeof(protocols) / sizeof(protocols[0])
};


bool available(void)
{
    return nReceivedValue != 0;
}

void reset_available(void)
{
    nReceivedValue = 0;
}


unsigned long long get_received_value(void)
{
    return nReceivedValue;
}

unsigned int get_received_bitlength(void)
{
    return nReceivedBitlength;
}

unsigned int get_received_delay(void)
{
    return nReceivedDelay;
}

unsigned int get_received_protocol(void)
{
    return nReceivedProtocol;
}

int get_received_tolerance(void)
{
    return nReceiveTolerance;
}


uint8_t get_num_protos(void)
{
    return numProto;
}

/**
 *
 */
bool receive_protocol(const int p, unsigned int changeCount)
{
    // FIXME: do we copy to ram from flash so check is faster in loops below?
    __xdata struct Protocol pro;

    // FIXME: we should probably check for out of bound index e.g. p = 0
    memcpy(&pro, &protocols[p-1], sizeof(struct Protocol));

    unsigned long long code = 0;
    unsigned int FirstTiming = 0;
    if (pro.PreambleFactor > 0) {
      FirstTiming = pro.PreambleFactor + 1;
    }
    unsigned int BeginData = 0;
    if (pro.HeaderFactor > 0) {
      BeginData = (pro.invertedSignal) ? (2) : (1);
      // Header pulse count correction for more than one
      if (pro.HeaderFactor > 1) {
        BeginData += (pro.HeaderFactor - 1) * 2;
      }
    }
    //Assuming the longer pulse length is the pulse captured in timings[FirstTiming]
    // берем наибольшее значение из Header
    const unsigned int syncLengthInPulses =  ((pro.Header.low) > (pro.Header.high)) ? (pro.Header.low) : (pro.Header.high);
    // определяем длительность Te как длительность первого импульса header деленную на количество импульсов в нем
    // или как длительность импульса preamble деленную на количество Te в нем
    unsigned int sdelay = 0;
    if (syncLengthInPulses > 0) {
      sdelay = timings[FirstTiming] / syncLengthInPulses;
    } else if (pro.PreambleFactor > 0) {
      sdelay = timings[FirstTiming-2] / pro.PreambleFactor;
    }
    const unsigned int delay = sdelay;
    // nReceiveTolerance = 60
    // допустимое отклонение длительностей импульсов на 60 %
    const unsigned int delayTolerance = delay * nReceiveTolerance / 100;

    // 0 - sync перед preamble или data
    // BeginData - сдвиг на 1 или 2 от sync к preamble/data
    // FirstTiming - сдвиг на preamble к header
    // firstDataTiming первый импульс data
    // bitChangeCount - количество импульсов в data

    /* For protocols that start low, the sync period looks like
     *               _________
     * _____________|         |XXXXXXXXXXXX|
     *
     * |--1st dur--|-2nd dur-|-Start data-|
     *
     * The 3rd saved duration starts the data.
     *
     * For protocols that start high, the sync period looks like
     *
     *  ______________
     * |              |____________|XXXXXXXXXXXXX|
     *
     * |-filtered out-|--1st dur--|--Start data--|
     *
     * The 2nd saved duration starts the data
     */
    // если invertedSignal=false, то сигнал начинается с 1 элемента массива (высокий уровень)
    // если invertedSignal=true, то сигнал начинается со 2 элемента массива (низкий уровень)
    // добавляем поправку на Преамбулу и Хедер
    const unsigned int firstDataTiming = BeginData + FirstTiming;
    unsigned int bitChangeCount = changeCount - firstDataTiming - 1 + pro.invertedSignal;
    if (bitChangeCount > 128) {
      bitChangeCount = 128;
    }

    for (unsigned int i = firstDataTiming; i < firstDataTiming + bitChangeCount; i += 2) {
        code <<= 1;
        if (abs(timings[i] - delay * pro.zero.high) < delayTolerance &&
            abs(timings[i + 1] - delay * pro.zero.low) < delayTolerance) {
            // zero
        } else if (abs(timings[i] - delay * pro.one.high) < delayTolerance &&
                   abs(timings[i + 1] - delay * pro.one.low) < delayTolerance) {
            // one
            code |= 1;
        } else {
            // Failed
            return false;
        }
    }

    if (bitChangeCount > 14) {    // ignore very short transmissions: no device sends them, so this must be noise
        nReceivedValue = code;
        nReceivedBitlength = bitChangeCount / 2;
        nReceivedDelay = delay;
        nReceivedProtocol = p;
        return true;
    }

    return false;
}