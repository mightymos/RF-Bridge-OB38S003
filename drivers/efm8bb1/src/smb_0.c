/**************************************************************************//**
 * Copyright (c) 2015 by Silicon Laboratories Inc. All rights reserved.
 *
 * http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt
 *****************************************************************************/

#include "smb_0.h"
#include "assert.h"

#define ALL_FLAGS  SMB0_TXMODE_SF | SMB0_START_SF | SMB0_STOP_SF | SMB0_ACKREQ_SF | SMB0_ARBLOST_SF | SMB0_ACK_SF

bool SMB0_getIntFlag(void)
{
  return SI;
}

void SMB0_clearIntFlag(void)
{
  SI = 0;
}

uint8_t SMB0_getStatusFlag(uint8_t flag) __reentrant
{
  return SMB0CN0 & flag;
}

void SMB0_setStatusFlag(uint8_t flag, uint8_t state) __reentrant
{
  if (state)
  {
    SMB0CN0 |= flag;
  }
  else
  {
    SMB0CN0 &= ~flag;
  }
}

uint8_t SMB0_read(void)
{
  return SMB0DAT;
}

void SMB0_write(uint8_t value)
{
  SMB0DAT = value;
}

void SMB0_abort(void)
{
  SMB0CF &= ~ENSMB__BMASK;
  SMB0CF |= ENSMB__BMASK;
  SMB0CN0 &= ~(STA__BMASK | STO__BMASK | ACKRQ__BMASK);
}

void SMB0_ack(bool ack)
{
  ACKRQ = ack;
}

void SMB0_start(void)
{
  STA = 1;
}

void SMB0_stop(void)
{
  STO = 1;
}

void SMB0_init(SMB0_Timebase_t timebase, bool sclLow) {
  SMB0CF &= ~(SMBCS__FMASK | SMBTOE__BMASK);
  SMB0CF |= timebase | ENSMB__ENABLED | INH__SLAVE_DISABLED | SMBFTE__FREE_TO_ENABLED | (uint8_t) sclLow << SMBTOE__SHIFT;
  SMB0ADM |= EHACK__ADR_ACK_AUTOMATIC;
}

void SMB0_initSlaveAddress(uint8_t address, uint8_t mask) {
  SMB0CF  &= ~INH__BMASK;
  SMB0ADR = address;
  SMB0ADM = mask | EHACK__BMASK;
}

void SMB0_reset(void)
{
  SMB0CF  = 0x0;
  SMB0TC  = 0x0;
  SMB0CN0 = 0x0;
  SMB0ADR = 0x0;
  SMB0ADM = 0x0;
}

#if EFM8PDL_SMB0_USE_STATEMACHINE == 1

void SMB0_ISR(void) __interrupt (SMBUS0_VECTOR)
{
  if(SMB0CN0_ARBLOST) {
    SMB0_arbLostCb(SMB0CN0 & 0xF0);
  }

  // Normal operation
  switch (SMB0CN0 & 0xF0)// Status vector
  {
    // SMB0CN0_MASTER ---------------------------------------------------------------

    // Master Transmitter/Receiver: START condition transmitted.
    case SMB0_MASTER_START:
      SMB0_startCb();
      SMB0CN0_STA = 0;
      break;

    // Master Transmitter: Data byte transmitted
    case SMB0_MASTER_TXDATA:
      SMB0_TxDataCb(true, SMB0CN0_ACK);
      break;

    // Master Receiver: byte received
    case SMB0_MASTER_RXDATA:
      SMB0_RxDataCb(true);
      break;

    case SMB0_SLAVE_TXDATA:
      SMB0_TxDataCb(false, SMB0CN0_ACK);
      break;

    case SMB0_SLAVE_ADDRESS:
      if(SMB0CN0_ARBLOST)
      {
        SMB0_arbLostCb(SMB0CN0 & 0xF0);
      }
      else
      {
        SMB0_addressCb();
      }
      SMB0CN0_STA = 0;  // Clear SMB0CN0_STA bit
      break;
                                      // Slave Receiver: Data received
    case SMB0_SLAVE_RXDATA:
      SMB0_RxDataCb(false);
      break;

                                      // Slave Receiver: Stop received while either a Slave Receiver or Slave
                                      // Transmitter
    case SMB0_SLAVE_RX_STOP:
    case SMB0_SLAVE_TX_STOP:
      SMB0_stopCb();
      SMB0CN0_STO = 0;
      break;

    default:
      break;

  } // end switch

  //Need an SMBus Restart?
  SMB0CN0_SI = 0;// Clear interrupt flag
}
#endif //EFM8PDL_SMB0_USE_STATEMACHINE

#if EFM8PDL_SMB0_USE_BUFFER == 1

__xdata uint8_t mAddress;
__xdata uint8_t mTxCount;
__xdata uint8_t mRxCount;
__xdata uint8_t* mTxBuffer;
__xdata uint8_t* mRxBuffer;

__xdata uint8_t sRxSize;
__xdata uint8_t sRxCount;
__xdata uint8_t sTxCount;
__xdata uint8_t* sRxBuffer;
__xdata uint8_t* sTxBuffer;

bool smbBusy;
bool smbReq;
bool smbReceive;
__xdata uint8_t mRetries;

void SMB0_transfer(uint8_t address, __xdata uint8_t* txBuffer, __xdata uint8_t* rxBuffer, uint8_t tx_len, uint8_t rx_len)
{
  //Setup transfer
  mAddress = address;
  mTxBuffer = txBuffer;
  mRxBuffer = rxBuffer;
  mTxCount = tx_len;
  mRxCount = rx_len;
  mRetries = EFM8PDL_SMB0_MASTER_RETRIES;

  if (smbBusy) {
    smbReq = true;
  } else {
    //Start transfer
    STA = 1;
  }
}

void SMB0_abortTransfer(void)
{
  mTxCount = 0;
  mRxCount = 0;
  sTxCount = 0;
  
  smbBusy = false;
  smbReq = false;
}

uint8_t SMB0_txBytesRemaining(void)
{
  return mTxCount;
}

uint8_t SMB0_rxBytesRemaining(void)
{
  return mRxCount;
}

void SMB0_initSlave(uint8_t address, __xdata uint8_t* commandBuffer, uint8_t length)
{
  SMB0CF  &= ~INH__BMASK;
  SMB0ADR = address;
  SMB0ADM = 0xFF | EHACK__BMASK;
  sRxBuffer = commandBuffer;
  sRxSize = length;
  sRxCount = 0;
}

uint8_t SMB0_getCommandLength(void)
{
  return sRxCount;
}

void SMB0_sendResponse(__xdata uint8_t* dataBuffer, uint8_t length)
{
  sTxBuffer = dataBuffer;
  sTxCount = length;
}

//SI_INTERRUPT_PROTO(SMB0_ISR, SMBUS0_IRQn);

void SMB0_ISR(void) __interrupt (SMBUS0_VECTOR)
{

  // Jump to status vector handler
  switch (SMB0CN0 & 0xF0)
  {
  // Master Transmitter/Receiver: START condition transmitted.
  case SMB0_MASTER_START:
    //Send address. If no tx data then initiate read.
    SMB0DAT = mAddress;
    if (!mTxCount)
    {
      SMB0DAT |= 0x01;
    }
    smbReceive = SMB0DAT & 0x01;

    //tailchain: clear_start;
    STA = 0;
    smbBusy = true;
    break;

  // Master Transmitter: Data byte transmitted
  case SMB0_MASTER_TXDATA:
    if (ACK)
    {
      if (smbReceive)
      {
        //Handle read transfer
        if(!--mRxCount)
        {
          ACK = 0; //ack for first data received byte
        }
        //ACK = 1 by default so no need to set it
      }
      else
      {
        //Handle writes
        if (mTxCount)
        {
          //if data remains in tx buffer send next byte
          SMB0DAT = *mTxBuffer++;
          mTxCount--;
        }
        else
        {
          //Handle writes finished
          if (mRxCount)
          {
            //if tx done and rx remaining do repeated start
            STA = 1;
          }
          else
          {
            //if tx done and no rx then stop
            //tailchain: stop_seq
            SMB0_transferCompleteCb();
            STO = 1;
            STA = smbReq;
            smbReq = false;
            smbBusy = false;
          } // if rxCount else
        }   //if txCount else
      }     //if ! rxReceive (rxReceive = ack of address so ignore)
    }       // if ACK
    else
    {
      //Error on NAC
      SMB0_errorCb(SMB0_NACK_ERROR);

      //tailchain: stop_seq
      STO = 1;
      STA = smbReq; //necessary for tailchain
      smbReq = false; //necessary for tailchain
      smbBusy = false;
    }
    break;

  // Master Receiver: byte received
  case SMB0_MASTER_RXDATA:
    //Read in data we just received
    *mRxBuffer++ = SMB0DAT;

    //if bytes remain read this one and set up for next one
    if (mRxCount)
    {
      //NACK if the byte we are about to read is our last
      if (!--mRxCount)
      {
        ACK = 0;
      }
      //ACK =1  by default so no need to set it.
    }
    else
    {
      ACK = 0;

      // If no bytes remain notify user xfer complete and issue stop.
      //tailchain: stop_seq
      SMB0_transferCompleteCb();
      STO = 1;
      STA = smbReq;
      smbReq = false;
      smbBusy = false;
    }
    break;

  // Slave Transmitter: Data byte transmitted
  case SMB0_SLAVE_TXDATA:
    if(ACK)
    {
      if(--sTxCount)
      {
        SMB0DAT = *sTxBuffer++;
      }
      else
      {
        SMB0_errorCb(SMB0_TXUNDER_ERROR);
      }
    }
    break;

  // Slave Receiver: Start+Address received
  case SMB0_SLAVE_ADDRESS:
    if(ARBLOST)
    {
      if(mRetries--)
      {
        smbReq = true;
      }
      else
      {
        SMB0_errorCb(SMB0_ARBLOST_ERROR);
      }
    }

    smbReceive = !(SMB0DAT & 0x01);
    // Nack defaults to 1 so no need to set it

    //Clear receive count if we are about to receive OR
    // if we have no rx phase (we are a write only xfer)
    // We leave rxCount alone if this is a repeated start
    if(smbReceive || !smbBusy)
    {
      sRxCount = 0;
    }

    // If this is a read then fire command received
    // either a read or a repeated start has been requested
    if (!smbReceive) {
    	// If rx buffer overflowed then commandReceivedCb all ready fired
    	if(sRxCount || !smbBusy){
        //Issue a command received callback if there is RX data or if this
        // is not a repeated start
        SMB0_commandReceivedCb();
      }

      SMB0DAT = *sTxBuffer++;
      //DO NOT dec sTxSize This is handled AFTER the first byte is transmitted
    }

    //tailchain: clear_start
    STA = 0;
    smbBusy = true;
    break;

  // Slave Receiver: Data received
  case SMB0_SLAVE_RXDATA:
    //Read data into buffer
    sRxBuffer[sRxCount++] = SMB0DAT;

    //If buffer is about to overflow callback user and reset buffer
    if (sRxCount == sRxSize)
    {
      SMB0_commandReceivedCb();
      sRxCount = 0;
    }

    //ACK = 1 by default so no need to set it
    break;

  // Slave Receiver: Stop received while either a Slave Receiver or Transmitter
  case SMB0_SLAVE_RX_STOP:
  case SMB0_SLAVE_TX_STOP:
    if (smbReceive && sRxCount)
    {
    	//Fire callback only if there is data in the buffer
      SMB0_commandReceivedCb();
    }

    //clear stop
    STO = 0;

    //If master request pending send a start.
    //tailchain: stop_seq
    STA = smbReq;
    smbReq = false;
    smbBusy = false;
    break;

  default:
    // All other cases are an error
    SMB0_errorCb(SMB0_UNKNOWN_ERROR);
    // ----------------------------------------------------------------------
  } // end switch

  //Clear interrupt flag
  SI = 0;

}

#endif //EFM8PDL_SMB0_USE_BUFFER
