/**
 * Sets Repeat Transmits
 */
void setRepeatTransmit(const int repeat)
{
  nRepeatTransmit = repeat;
}

/**
  * Sets the protocol to send.
  */
void setProtocol(const struct Protocol pro)
{
  protocol = pro;
}

/**
  * Sets the protocol to send, from a list of predefined protocols
  */
void setProtocol(const int nProtocol)
{
    if (nProtocol < 1 || nProtocol > numProto)
    {
        // TODO: trigger an error, e.g. "bad protocol" ?
        nProtocol = 1;
    }

	memcpy(&protocol, &protocols[nProtocol-1], sizeof(struct Protocol));
}

/**
 * Transmit a single high-low pulse.
 */
void transmit(Struct Protocol *pro, HighLow pulses)
{
  uint8_t firstLogicLevel  = (pro->inverted) ? 0 : 1;
  uint8_t secondLogicLevel = (pro->inverted) ? 1 : 0;
  
    if (firstLogicLevel)
    {
        tdata_on();
    } else {
        tdata_off();
    }

    delay_us(pro->pulseLength * pulses.high);

    if (secondLogicLevel)
    {
        tdata_on();
    } else {
        tdata_off();
    }

    delay_us(pro->pulseLength * pulses.low);
}

/**
 * @param sCodeWord   a binary code word consisting of the letter 0, 1
 */
void send(const char* sCodeWord) {

  // turn the tristate code word into the corresponding bit pattern, then send it
  unsigned long code = 0;
  unsigned int length = 0;

  for (const char* p = sCodeWord; *p; p++) {
    code <<= 1L;
    if (*p != '0')
      code |= 1L;

    length++;
  }

  send(code, length);
}


/**
 * Transmit the first 'length' bits of the integer 'code'. The
 * bits are sent from MSB to LSB, i.e., first the bit at position length-1,
 * then the bit at position length-2, and so on, till finally the bit at position 0.
 */
void send(const int nProtocol, unsigned long code, unsigned int length)
{
    // FIXME: it might just be easier to make this global
    // and possibly share with receive protocol, if they are never used at the same time
    struct Protocol pro;

    // also checks for out of bound index (e.g., less than one)
	//setProtocol(nProtocol);

    // FIXME: consider checking index out of bound
    memcpy(&pro, &protocols[nProtocol-1], sizeof(struct Protocol));

	// make sure the receiver is disabled while we transmit
	radio_receiver_off();

	for (int nRepeat = 0; nRepeat < nRepeatTransmit; nRepeat++)
    {
		for (int i = length-1; i >= 0; i--)
        {
		    if (code & (1L << i))
	      	{
			    transmit(&pro, pro.one);
	      	}
		    else
	      	{
			    transmit(&pro, pro.zero);
	      	}
	    }

		transmit(pro.syncFactor);
	}

	// disable transmit after sending (i.e., for inverted protocols)
	tdata_off();

	// enable receiver again if we just disabled it
	radio_receiver_on();
}