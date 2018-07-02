
void bcm_set_pinmux(unsigned int pin_num, unsigned int mux_num);
int bcm_init_pinmux(void);
void bcm_init_pinmux_interface(unsigned int interface);
#if 1 /* R8000P BRCM PATCH */
void bcm_hw_led_disable(void);
#endif


