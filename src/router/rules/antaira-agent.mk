aiccu-configure:
	@true

aiccu:
	make -C antaira-agent

aiccu-clean:
	make -C antaira-agent clean

aiccu-install:
	make -C antaira-agent install
