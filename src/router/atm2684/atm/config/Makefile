noall:
	@echo "Please specify the type of distribution you're using as the"; \
	echo "make target, e.g. make RedHat-4.0  The following Linux"; \
	echo "distribution are currently supported:"; \
	echo; \
	echo "  RedHat-4.0"; \
	echo; \
	echo "(Note that names are case-sensitive)"

RedHat-4.0:
	cd common && make install
	cd redhat-4.0 && make install
