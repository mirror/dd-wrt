#
# OCTEON-specific crypto modules.
#

obj-y += octeon-crypto.o

obj-$(CONFIG_CRYPTO_MD5_OCTEON)		+= octeon-md5.o
obj-$(CONFIG_CRYPTO_SHA1_OCTEON)	+= octeon-sha1.o
obj-$(CONFIG_CRYPTO_SHA256_OCTEON)	+= octeon-sha256.o
obj-$(CONFIG_CRYPTO_SHA512_OCTEON)	+= octeon-sha512.o
obj-$(CONFIG_CRYPTO_AES_OCTEON)		+= octeon-aes.o octeon-aes-cbc.o
obj-$(CONFIG_CRYPTO_GHASH_OCTEON)	+= octeon-aes-gcm.o
obj-$(CONFIG_CRYPTO_CRC32_OCTEON)	+= octeon-crc32c.o
octeon-crc32c-objs			:= octeon-crc32.o crc-switch.o
