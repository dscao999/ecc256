
.phony: all clean release

CFLAGS += -fPIC

all:	rnda sha ecc aes crc b64tx ripe

rnda: capture.o alsa_random.o sha256.o
	$(LINK.o) $^ -lasound -o $@

sha: sha.o sha256.o base64.o
	$(LINK.o) $^ -o $@

clean:
	rm -f *.o rnda sha ecc aes crc crctbl b64tx ripe

ecc: ecc.o ecc_secp256k1.o alsa_random.o sha256.o dscrc.o base64.o
	$(LINK.o) $^ -lgmp -lasound -o $@

aes: aes.o dsaes.o
	$(LINK.o) $^ -o $@

crc: crc.o dscrc.o
	$(LINK.o) $^ -lreadline -o $@

#crctbl: crc_table.o
#	$(LINK.o) $^ -o $@

b64tx: b64tx.o alsa_random.o sha256.o base64.o
	$(LINK.o) $^ -lasound -o $@

ripe: ripemd.o ripemd160.o base64.o
	$(LINK.o) $^ -o $@

release: rnda sha ecc ripe crc

release: CFLAGS += -O2
release: LDFLAGS += -Wl,-O2
