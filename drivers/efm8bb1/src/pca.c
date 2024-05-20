void PCA0_run(void)
{
    PCA0CN0 |= CR__RUN;
}

void PCA0_halt(void)
{
    PCA0CN0 &= ~CR__RUN;
}

void PCA0_channel0EventCb(void)
{
    //FIXME: possible to eliminate multiplication to save code size?
	uint16_t current_capture_value = PCA0CP0 * 10;
	uint8_t flags = PCA0MD;

	// clear counter
	PCA0MD &= 0xBF;
	PCA0H = 0x00;
	PCA0L = 0x00;
	PCA0MD = flags;


	// FIXME: additional comments; if bucket is not noise add it to buffer
	if (current_capture_value < 0x8000)
	{
		// FIXME: add comment
		buffer_in(current_capture_value | ((uint16_t)(!rdata_level()) << 15));
	}
	else
	{
		// received noise, so clear all received buckets
		buffer_buckets_read = 0;
		buffer_buckets_write = 0;
	}
}



void PCA0_ISR(void) __interrupt (PCA0_VECTOR)
{
  // save and clear flags
  uint8_t flags = PCA0CN0 & (CF__BMASK | CCF0__BMASK | CCF1__BMASK | CCF2__BMASK);

  PCA0CN0 &= ~flags;

  if( (PCA0PWM & COVF__BMASK) && (PCA0PWM & ECOV__BMASK))
  {
    PCA0_intermediateOverflowCb();
  }

  PCA0PWM &= ~COVF__BMASK;

  if((flags & CF__BMASK) && (PCA0MD & ECF__BMASK))
  {
    PCA0_overflowCb();
  }

  if((flags & CCF0__BMASK) && (PCA0CPM0 & ECCF__BMASK))
  {
  	// apparently our radio input
    PCA0_channel0EventCb();
  }

  if((flags & CCF1__BMASK) && (PCA0CPM1 & ECCF__BMASK))
  {
    PCA0_channel1EventCb();
  }

  if((flags & CCF2__BMASK) && (PCA0CPM2 & ECCF__BMASK))
  {
    PCA0_channel2EventCb();
  }
}