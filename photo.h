#ifndef photo_h
#define photo_h

#include "bmp.h"
#include "aes.h"

int photo(char clee_hex[], int longueur_clee, int mode_cbc, unsigned char iv[16]);

#endif