dist_man_MANS = bt-adapter.1 \
		bt-agent.1 \
		bt-audio.1 \
		bt-device.1 \
		bt-network.1 \
		bt-obex.1

.pod.1:
	$(POD2MAN) -c "General Commands Manual" -r "$(PACKAGE_NAME) $(PACKAGE_VERSION)" $^ >$@
