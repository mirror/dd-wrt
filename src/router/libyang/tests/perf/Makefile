ITEMS=5000
CFLAGS=-Wall -O0

compilation: validation validation_xml addloop

all: addloop validation validation_xml sizes test

addloop: addloop.c
	$(CC) $(CFLAGS) -lyang $< -o $@

validation: validation.c
	$(CC) $(CFLAGS) -lyang $< -o $@

validation_xml: validation_xml.c
	$(CC) $(CFLAGS) -lxml2 -lxslt $< -o $@

sizes: sizes.c ../../src/tree_schema.h ../../src/tree_data.h
	$(CC) $(CFLAGS) $< -o $@

test: addloop validation validation_xml
	@rm -rf data.xml data_xml.xml addloop_result.xml; \
	echo "Adding 5000 list items one by one (libyang)"; \
	TIME=" time  : %Es\n memory: %MKb" time ./addloop perftest.yin | grep real | sed 's/* //'; \
	echo;
	@for i in $(shell seq 1 ${ITEMS}); do \
		echo "<ptest1 xmlns=\"urn:libyang:performance:test\"><index>$$i</index><p1>$$i</p1></ptest1>" >> data.xml; \
	done;
	@echo "<config xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">" > data_xml.xml; \
	cat data.xml >> data_xml.xml; \
	echo "</config>" >> data_xml.xml; \
	echo "Loading and validating data with $(ITEMS) items..."; \
	echo "libyang"; \
	TIME=" time  : %Es\n memory: %MKb" time ./validation perftest.yin data.xml | grep real | sed 's/* //'; \
	echo; \
	echo "libxml2"; \
	TIME=" time  : %Es\n memory: %MKb" time ./validation_xml perftest.yin data_xml.xml perftest-config.rng perftest-schematron.xsl; \

clean:
	rm -rf sizes validation validation_xml addloop data.xml data_xml.xml addloop_result.xml

