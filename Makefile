CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -O2 -fopenmp -maes -msse4.1
LDLIBS  = -lm

COMMUN  = aes_encrypt.c aes_decrypt.c aes_ni.c bmp.c fichier.c fonctions.c photo.c
HEADERS = $(wildcard *.h)

# Outil interactif + tests sur les vecteurs officiels (FIPS-197)
all: aes test_aes

aes: main.c $(COMMUN) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ main.c $(COMMUN) $(LDLIBS)

test_aes: test.c $(COMMUN) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ test.c $(COMMUN) $(LDLIBS)

test: test_aes
	./test_aes

clean:
	rm -f aes test_aes

.PHONY: all test clean
