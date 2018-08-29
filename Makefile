
.phony: all clean release

LDFLAGS += -g
CFLAGS += -g

all:	rnda sha ecc aes crc

rnda: capture.o alsa_random.o digest.o
	$(LINK.o) $(LDFLAGS) $^ -lasound -o $@

sha: sha256.o digest.o
	$(LINK.o) $(LDFLAGS) $^ -o $@

clean:
	rm -f *.o rnda sha ecc aes crc

ecc: ecc.o ecc_secp256k1.o alsa_random.o digest.o
	$(LINK.o) $(LDFLAGS) $^ -lgmp -lasound -o $@

aes: aes.o dsaes.o
	$(LINK.o) $(LDFLAGS) $^ -o $@

crc: crc.o dscrc.o
	$(LINK.o) $(LDFLAGS) $^ -o $@

release: rnda sha ecc

release: CFLAGS += -O2
release: LDFLAGS += -O2
