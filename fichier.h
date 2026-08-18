#ifndef aes_fichier_h
#define aes_fichier_h

#include "aes.h"
#include "fonctions.h"



int aes_ecb_encrypt(unsigned char** dst, int *dst_data_length, const unsigned char* src,
int src_data_length, const uint8_t key[], int key_length);

int aes_ecb_decrypt(unsigned char** dst, int *dst_data_length, const unsigned char* src,
int src_data_length, const uint8_t key[], int key_length);
int aes_cbc_encrypt(unsigned char** dst, int *dst_data_length, const unsigned char* src,
                    int src_data_length, const unsigned char* iv, const uint8_t key[], int key_length);

int aes_cbc_decrypt(unsigned char** dst, int *dst_data_length, const unsigned char* src,
                    int src_data_length, const unsigned char* iv, const uint8_t key[], int key_length);

int aes_fichier(char clee_hex[], int longueur_clee, int mode_cbc, unsigned char iv[16]);

#endif