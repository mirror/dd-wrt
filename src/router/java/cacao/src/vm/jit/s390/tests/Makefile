all: install

tests.java: tests.java.pp
	cpp tests.java.pp | \
	sed -e '/__JAVA_LINE__/{=;d};s.^#.// cpp: .' > tests.java

tests.class: tests.java
	ecj -bootclasspath /home/peter/cacao-dev/cldc-classes $<

install: tests.class
	cp *.class /home/peter/cacao-dev/cldc-classes/tests

clean:
	rm *.class tests.java
