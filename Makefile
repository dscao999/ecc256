
.phony: all clean release

CFLAGS ?= -Wall -g
CFLAGS += -fPIC
LDFLAGS += -g

all:	rnda sha ecc aes crc b64tx ripe

srcs = $(wildcard *.c)
deps = $(srcs:.c=.d)

rnda: capture.o alsarec.o sha256.o
	$(LINK.o) $^ -lasound -o $@

sha: sha.o sha256.o base64.o
	$(LINK.o) $^ -o $@

clean:
	rm -f *.o rnda sha ecc aes crc crctbl b64tx ripe
	rm -f $(deps)

ecc: ecc.o ecc_secp256k1.o alsarec.o sha256.o dscrc.o base64.o \
	dsaes.o ripemd160.o
	$(LINK.o) $^ -lgmp -lasound -o $@

aes: aes.o dsaes.o
	$(LINK.o) $^ -o $@

crc: crc.o dscrc.o
	$(LINK.o) $^ -lreadline -o $@

#crctbl: crc_table.o
#	$(LINK.o) $^ -o $@

b64tx: b64tx.o alsarec.o sha256.o base64.o
	$(LINK.o) $^ -lasound -o $@

ripe: ripemd.o ripemd160.o base64.o
	$(LINK.o) $^ -o $@

release: rnda sha ecc ripe crc

release: CFLAGS += -O2
release: LDFLAGS += -Wl,-O2

%.o: %.c
	$(COMPILE.c) -MMD -MP -c $< -o $@

-include $(deps)
