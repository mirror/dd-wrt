/**
 * returns abs(t2-t1)
 *
**/
static inline cycles_t substract_cycles(cycles_t t2, cycles_t t1) {
	return (t2 > t1)?(t2 - t1):(t1 - t2);
}

/**
 * wait for a delay expressed in cycles_t, cycles of CPU
 *
**/
static inline void cycles_delay(cycles_t delay) {
	cycles_t now = get_cycles();
	cycles_t future = now + delay;	/* can overflow and wrap arround */
	if (future < now) {
		// if wrapped arround wait till now <= future
		while (get_cycles() > future) ;
	}
	
	while (get_cycles() < future) ;
}

/**
 * if *ref == 0 => wait delta_c
 * else:
 * wait till NOW is *ref + delta_c
 * when done, change *ref to NOW
**/
static void wait_delta_cycles(cycles_t *ref, cycles_t delta_c) {
	if (*ref == 0) {
		// function used for the first time
		cycles_delay(delta_c);
	} else {
		cycles_t delta_clk = substract_cycles(get_cycles(), *ref);
		
		if (delta_clk < delta_c) {
			// we have to pause ...
			cycles_delay(delta_c - delta_clk);
		}
	}
	
	*ref=get_cycles();
	if (*ref == 0) {
		// we use 0 to tell that this function is used for the first time
		// 0 is then forbidden.
		*ref++;
	}
}


static inline void mmc_spi_cs_low(void) {
  port_state &= ~(SD_CS);
  ps_di =(port_state|SD_DI);
  ps_di_clk =(port_state|DI_CLK);
  ps_clk =(port_state|SD_CLK);
  *gpioaddr_output = port_state;
}

static inline void mmc_spi_cs_high(void) {
  port_state |= SD_CS;
  ps_di =(port_state|SD_DI);
  ps_di_clk =(port_state|DI_CLK);
  ps_clk =(port_state|SD_CLK);
  *gpioaddr_output = port_state;
}

static inline void mmc_spi_clk(void) {
	// Send one CLK to SPI with DI high
	*gpioaddr_output = ps_di;
	*gpioaddr_output = ps_di_clk;
}

/* send the clock to the card with a maximum clock period of hfp<<1 in CPU cycles) */
static inline void mmc_spi_clk_fp(cycles_t *last_clk, cycles_t hfp) {
	// Send one CLK to SPI with DI high
	wait_delta_cycles(last_clk, hfp);
	*gpioaddr_output = ps_di;
	wait_delta_cycles(last_clk, hfp);
	*gpioaddr_output = ps_di_clk;
}

static inline void mmc_spi_io_ff_v(void) {
	const unsigned char l_ps_di = ps_di;
	const unsigned char l_ps_di_clk = ps_di_clk;
	volatile unsigned char * const l_gpioaddr_output = (unsigned char *) gpioaddr_output;
	
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
}

static inline void mmc_spi_io_ff_v_fp(cycles_t *last_clk, cycles_t hfp) {
	const unsigned char l_ps_di = ps_di;
	const unsigned char l_ps_di_clk = ps_di_clk;
	volatile unsigned char * const l_gpioaddr_output = (unsigned char *) gpioaddr_output;
	
	*l_gpioaddr_output = l_ps_di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_di_clk; wait_delta_cycles(last_clk, hfp);
	*l_gpioaddr_output = l_ps_di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_di_clk; wait_delta_cycles(last_clk, hfp);
	*l_gpioaddr_output = l_ps_di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_di_clk; wait_delta_cycles(last_clk, hfp);
	*l_gpioaddr_output = l_ps_di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_di_clk; wait_delta_cycles(last_clk, hfp);
	*l_gpioaddr_output = l_ps_di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_di_clk; wait_delta_cycles(last_clk, hfp);
	*l_gpioaddr_output = l_ps_di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_di_clk; wait_delta_cycles(last_clk, hfp);
	*l_gpioaddr_output = l_ps_di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_di_clk; wait_delta_cycles(last_clk, hfp);
	*l_gpioaddr_output = l_ps_di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_di_clk; wait_delta_cycles(last_clk, hfp);
}

static inline unsigned char mmc_spi_io_ff(void) {
	const unsigned char l_ps_di = ps_di;
	const unsigned char l_ps_di_clk = ps_di_clk;
	volatile unsigned char * const l_gpioaddr_output = (unsigned char *) gpioaddr_output;
	volatile unsigned char * l_gpioaddr_input = (unsigned char *) gpioaddr_input;
	unsigned char result = 0;
	
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
	GET_RESULT_DO(SHIFT_DO)
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
	GET_RESULT_DO(SHIFT_DO-1)
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
	GET_RESULT_DO(SHIFT_DO-2)
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
	GET_RESULT_DO(SHIFT_DO-3)
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
	GET_RESULT_DO(SHIFT_DO-4)
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
	GET_RESULT_DO(SHIFT_DO-5)
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
	GET_RESULT_DO(SHIFT_DO-6)
	*l_gpioaddr_output = l_ps_di; *l_gpioaddr_output = l_ps_di_clk;
	GET_RESULT_DO(SHIFT_DO-7)
	
	return result;
}

static inline unsigned char mmc_spi_io_ff_fp(cycles_t *last_clk, cycles_t hfp) {
	const unsigned char l_ps_di = ps_di;
	const unsigned char l_ps_di_clk = ps_di_clk;
	volatile unsigned char * const l_gpioaddr_output = (unsigned char *) gpioaddr_output;
	volatile unsigned char * l_gpioaddr_input = (unsigned char *) gpioaddr_input;
	unsigned char result = 0;
	
	*l_gpioaddr_output = l_ps_di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_di_clk; wait_delta_cycles(last_clk, hfp);
	GET_RESULT_DO(SHIFT_DO)
	*l_gpioaddr_output = l_ps_di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_di_clk; wait_delta_cycles(last_clk, hfp);
	GET_RESULT_DO(SHIFT_DO-1)
	*l_gpioaddr_output = l_ps_di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_di_clk; wait_delta_cycles(last_clk, hfp);
	GET_RESULT_DO(SHIFT_DO-2)
	*l_gpioaddr_output = l_ps_di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_di_clk; wait_delta_cycles(last_clk, hfp);
	GET_RESULT_DO(SHIFT_DO-3)
	*l_gpioaddr_output = l_ps_di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_di_clk; wait_delta_cycles(last_clk, hfp);
	GET_RESULT_DO(SHIFT_DO-4)
	*l_gpioaddr_output = l_ps_di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_di_clk; wait_delta_cycles(last_clk, hfp);
	GET_RESULT_DO(SHIFT_DO-5)
	*l_gpioaddr_output = l_ps_di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_di_clk; wait_delta_cycles(last_clk, hfp);
	GET_RESULT_DO(SHIFT_DO-6)
	*l_gpioaddr_output = l_ps_di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_di_clk; wait_delta_cycles(last_clk, hfp);
	GET_RESULT_DO(SHIFT_DO-7)
	
	return result;
}

static inline void mmc_spi_io_v(const unsigned char data_out) {
	unsigned char di;
	const unsigned char l_port_state = port_state;
	const unsigned char l_ps_clk = ps_clk;
	volatile unsigned char * const l_gpioaddr_output = (unsigned char *) gpioaddr_output;

	SET_DI(0x80,SHIFT_DI)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
	SET_DI(0x40,SHIFT_DI-1)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
	SET_DI(0x20,SHIFT_DI-2)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
	SET_DI(0x10,SHIFT_DI-3)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
	SET_DI(0x08,SHIFT_DI-4)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
	SET_DI(0x04,SHIFT_DI-5)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
	SET_DI(0x02,SHIFT_DI-6)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
	SET_DI(0x01,SHIFT_DI-7)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
}

static inline void mmc_spi_io_v_fp(cycles_t *last_clk, cycles_t hfp, const unsigned char data_out) {
	unsigned char di;
	const unsigned char l_port_state = port_state;
	const unsigned char l_ps_clk = ps_clk;
	volatile unsigned char * const l_gpioaddr_output = (unsigned char *) gpioaddr_output;

	SET_DI(0x80,SHIFT_DI)
	*l_gpioaddr_output = l_port_state|di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_clk|di; wait_delta_cycles(last_clk, hfp);
	SET_DI(0x40,SHIFT_DI-1)
	*l_gpioaddr_output = l_port_state|di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_clk|di; wait_delta_cycles(last_clk, hfp);
	SET_DI(0x20,SHIFT_DI-2)
	*l_gpioaddr_output = l_port_state|di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_clk|di; wait_delta_cycles(last_clk, hfp);
	SET_DI(0x10,SHIFT_DI-3)
	*l_gpioaddr_output = l_port_state|di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_clk|di; wait_delta_cycles(last_clk, hfp);
	SET_DI(0x08,SHIFT_DI-4)
	*l_gpioaddr_output = l_port_state|di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_clk|di; wait_delta_cycles(last_clk, hfp);
	SET_DI(0x04,SHIFT_DI-5)
	*l_gpioaddr_output = l_port_state|di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_clk|di; wait_delta_cycles(last_clk, hfp);
	SET_DI(0x02,SHIFT_DI-6)
	*l_gpioaddr_output = l_port_state|di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_clk|di; wait_delta_cycles(last_clk, hfp);
	SET_DI(0x01,SHIFT_DI-7)
	*l_gpioaddr_output = l_port_state|di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_clk|di; wait_delta_cycles(last_clk, hfp);
}

#if 0
static unsigned char mmc_spi_io(unsigned char data_out) {
	unsigned char result = 0;
	unsigned char di;
	const unsigned char l_port_state = port_state;
	const unsigned char l_ps_clk= ps_clk;
	volatile unsigned char * const l_gpioaddr_output = (unsigned char *) gpioaddr_output;
	volatile unsigned char * const l_gpioaddr_input = (unsigned char *) gpioaddr_input;
		
	SET_DI(0x80,SHIFT_DI)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
	GET_RESULT_DO(SHIFT_DO)
	SET_DI(0x40,SHIFT_DI-1)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
	GET_RESULT_DO(SHIFT_DO-1)
	SET_DI(0x20,SHIFT_DI-2)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
	GET_RESULT_DO(SHIFT_DO-2)
	SET_DI(0x10,SHIFT_DI-3)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
	GET_RESULT_DO(SHIFT_DO-3)
	SET_DI(0x08,SHIFT_DI-4)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
	GET_RESULT_DO(SHIFT_DO-4)
	SET_DI(0x04,SHIFT_DI-5)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
	GET_RESULT_DO(SHIFT_DO-5)
	SET_DI(0x02,SHIFT_DI-6)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
	GET_RESULT_DO(SHIFT_DO-6)
	SET_DI(0x01,SHIFT_DI-7)
	*l_gpioaddr_output = l_port_state|di; *l_gpioaddr_output = l_ps_clk|di;
	GET_RESULT_DO(SHIFT_DO-7)
 
	return(result);
}

static unsigned char mmc_spi_io_fp(cycles_t *last_clk, cycles_t hfp, unsigned char data_out) {
	unsigned char result = 0;
	unsigned char di;
	const unsigned char l_port_state = port_state;
	const unsigned char l_ps_clk= ps_clk;
	volatile unsigned char * const l_gpioaddr_output = (unsigned char *) gpioaddr_output;
	volatile unsigned char * const l_gpioaddr_input = (unsigned char *) gpioaddr_input;
		
	SET_DI(0x80,SHIFT_DI)
	*l_gpioaddr_output = l_port_state|di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_clk|di; wait_delta_cycles(last_clk, hfp);
	GET_RESULT_DO(SHIFT_DO)
	SET_DI(0x40,SHIFT_DI-1)
	*l_gpioaddr_output = l_port_state|di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_clk|di; wait_delta_cycles(last_clk, hfp);
	GET_RESULT_DO(SHIFT_DO-1)
	SET_DI(0x20,SHIFT_DI-2)
	*l_gpioaddr_output = l_port_state|di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_clk|di; wait_delta_cycles(last_clk, hfp);
	GET_RESULT_DO(SHIFT_DO-2)
	SET_DI(0x10,SHIFT_DI-3)
	*l_gpioaddr_output = l_port_state|di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_clk|di; wait_delta_cycles(last_clk, hfp);
	GET_RESULT_DO(SHIFT_DO-3)
	SET_DI(0x08,SHIFT_DI-4)
	*l_gpioaddr_output = l_port_state|di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_clk|di; wait_delta_cycles(last_clk, hfp);
	GET_RESULT_DO(SHIFT_DO-4)
	SET_DI(0x04,SHIFT_DI-5)
	*l_gpioaddr_output = l_port_state|di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_clk|di; wait_delta_cycles(last_clk, hfp);
	GET_RESULT_DO(SHIFT_DO-5)
	SET_DI(0x02,SHIFT_DI-6)
	*l_gpioaddr_output = l_port_state|di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_clk|di; wait_delta_cycles(last_clk, hfp);
	GET_RESULT_DO(SHIFT_DO-6)
	SET_DI(0x01,SHIFT_DI-7)
	*l_gpioaddr_output = l_port_state|di; wait_delta_cycles(last_clk, hfp); *l_gpioaddr_output = l_ps_clk|di; wait_delta_cycles(last_clk, hfp);
	GET_RESULT_DO(SHIFT_DO-7)
 
	return(result);
}
#endif

static inline unsigned char mmc_spi_skip_nr_and_get_response(void) {
	unsigned char i, r;
	/* skip NR and get response */
	for (i = 0; i < 9; i++) {
		r = mmc_spi_io_ff();
		if (r != 0xff) break;
	}
	
	return r;
}

static inline void mmc_spi_send_word(unsigned int word) {
	mmc_spi_io_v(word >> 24);
	mmc_spi_io_v(word >> 16);
	mmc_spi_io_v(word >> 8);
	mmc_spi_io_v(word);
}

static inline void mmc_spi_send_word_fp(cycles_t *last_clk, cycles_t hfp, unsigned int word) {
	mmc_spi_io_v_fp(last_clk, hfp, word >> 24);
	mmc_spi_io_v_fp(last_clk, hfp, word >> 16);
	mmc_spi_io_v_fp(last_clk, hfp, word >> 8);
	mmc_spi_io_v_fp(last_clk, hfp, word);
}

static inline unsigned char mmc_spi_send_cmd(unsigned char cmd_id, unsigned int param) {
	mmc_spi_io_v(		0x40 | cmd_id);
	mmc_spi_send_word(	param);
	mmc_spi_io_ff_v();

	return mmc_spi_skip_nr_and_get_response();
}

/* sends a command with param 'param' then skip 'skip' bytes then get response */
static inline unsigned char mmc_spi_send_cmd_skip(unsigned char cmd_id, unsigned int param, unsigned char skip) {
	unsigned char i;
	mmc_spi_io_v(		0x40 | cmd_id);
	mmc_spi_send_word(	param);
	mmc_spi_io_ff_v();
	for (i=0; i<skip; i++) mmc_spi_io_ff_v();
	return mmc_spi_skip_nr_and_get_response();
}

static inline unsigned char mmc_spi_send_cmd_fp(cycles_t *last_clk, cycles_t hfp, unsigned char cmd_id, unsigned int param) {
	mmc_spi_io_v_fp(	last_clk, hfp, 0x40 | cmd_id);
	mmc_spi_send_word_fp(	last_clk, hfp, param);
	mmc_spi_io_ff_v_fp(	last_clk, hfp);
	
	return mmc_spi_skip_nr_and_get_response();
}

static inline unsigned char mmc_spi_send_cmd_crc7(unsigned char cmd_id, unsigned int param, unsigned char crc7) {
	mmc_spi_io_v(		0x40 | cmd_id);
	mmc_spi_send_word(	param);
	mmc_spi_io_v(		crc7);
	
	return mmc_spi_skip_nr_and_get_response();
}

static inline unsigned char mmc_spi_send_cmd_crc7_fp(cycles_t *last_clk, cycles_t hfp, unsigned char cmd_id, unsigned int param, unsigned char crc7) {
	mmc_spi_io_v_fp(	last_clk, hfp, 0x40 | cmd_id);
	mmc_spi_send_word_fp(	last_clk, hfp, param);
	mmc_spi_io_v_fp(	last_clk, hfp, crc7);
	
	return mmc_spi_skip_nr_and_get_response();
}
