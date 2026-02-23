# Nothing here is built or distributed by default.

all: serpent-sbox3.pdf serpent-sbox4.pdf serpent-sbox5.pdf \
     serpent-sbox6.pdf serpent-sbox7.pdf \
     serpent-sbox0i.pdf serpent-sbox1i.pdf serpent-sbox2i.pdf \
     serpent-sbox3i.pdf serpent-sbox4i.pdf serpent-sbox5i.pdf

%.pdf: %.dot
	dot -Tpdf $< -o $@
