
.phony: all clean release

LDFLAGS += -g
CFLAGS += -g

all:	rnda sha ecc aes

rnda: capture.o digest.o
	$(LINK.o) $(LDFLAGS) $^ -lasound -o $@

sha: sha256.o digest.o
	$(LINK.o) $(LDFLAGS) $^ -o $@

clean:
	rm -f *.o rnda sha

ecc: ecc.o ecc_secp256k1.o
	$(LINK.o) $(LDFLAGS) $^ -lgmp -o $@

aes: aes.o dsaes.o
	$(LINK.o) $(LDFLAGS) $^ -o $@

release: rnda sha ecc

release: CFLAGS += -O2
release: LDFLAGS += -O2
