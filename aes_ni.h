#ifndef AES_NI_H
#define AES_NI_H

#include <stdint.h>
#include <wmmintrin.h>

// Taille max du schedule : (nr+1) round keys de 128 bits chacune
// AES-256 : 15 round keys = 15 x __m128i
#define AES_NI_MAX_ROUNDS 14

typedef struct {
    __m128i enc[15];  // round keys pour chiffrement
    __m128i dec[15];  // round keys pour dechiffrement (InvMixColumns appliquee)
    int nr;           // nombre de tours (10, 12 ou 14)
} aes_ni_schedule_t;

int aes_ni_key_expansion(const uint8_t key[], int key_length_bytes, aes_ni_schedule_t *sched);

void aes_ni_encrypt_block(const uint8_t plain[16], uint8_t cipher[16], const aes_ni_schedule_t *sched);
void aes_ni_decrypt_block(const uint8_t cipher[16], uint8_t plain[16],  const aes_ni_schedule_t *sched);

int aes_ni_ecb_encrypt(unsigned char **dst, int *dst_len,
                       const unsigned char *src, int src_len,
                       const uint8_t key[], int key_length_bytes);
int aes_ni_ecb_decrypt(unsigned char **dst, int *dst_len,
                       const unsigned char *src, int src_len,
                       const uint8_t key[], int key_length_bytes);

#endif
