
extern void radio_off(int idx);
extern void radio_on(int idx);

void start_radio_off(void)
{
	radio_off(-1);
}

void start_radio_on(void)
{
	radio_on(-1);
}

void start_radio_off_0(void)
{
	radio_off(0);
}

void start_radio_on_0(void)
{
	radio_on(0);
}

void start_radio_off_1(void)
{
	radio_off(1);
}

void start_radio_on_1(void)
{
	radio_on(1);
}
