prepare:
	rm -rf httpd/ej_temp
	mkdir -p httpd/ej_temp
	cp $(TOP)/opt/etc/config/*.webconfig httpd/ej_temp

