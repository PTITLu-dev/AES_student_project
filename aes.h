#ifndef AES_H
#define AES_H

#include <stdint.h>

typedef uint8_t state[4][4];

void aes_encrypt(state dst, state src, const uint8_t key[], int nr);
void aes_decrypt(state dst, state src, const uint8_t key[], int nr);
void aes_encrypt_avec_clee_etendue(state dst, state src, const uint8_t clee_etendue[], int nr);
void aes_decrypt_avec_clee_etendue(state dst, state src, const uint8_t clee_etendue[], int nr);

#endif