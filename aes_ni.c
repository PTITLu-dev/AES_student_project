#include "aes_ni.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>



static __m128i expand256_step1(__m128i k0, __m128i k1, uint8_t rcon) {
    __m128i kg = _mm_aeskeygenassist_si128(k1, rcon);
    kg = _mm_shuffle_epi32(kg, 0xFF);
    k0 = _mm_xor_si128(k0, _mm_slli_si128(k0, 4));
    k0 = _mm_xor_si128(k0, _mm_slli_si128(k0, 4));
    k0 = _mm_xor_si128(k0, _mm_slli_si128(k0, 4));
    return _mm_xor_si128(k0, kg);
}

static __m128i expand256_step2(__m128i k0, __m128i k1) {
    __m128i kg = _mm_aeskeygenassist_si128(k0, 0x00);
    kg = _mm_shuffle_epi32(kg, 0xAA);
    k1 = _mm_xor_si128(k1, _mm_slli_si128(k1, 4));
    k1 = _mm_xor_si128(k1, _mm_slli_si128(k1, 4));
    k1 = _mm_xor_si128(k1, _mm_slli_si128(k1, 4));
    return _mm_xor_si128(k1, kg);
}

static void expand_256(const uint8_t key[32], __m128i enc[15]) {
    enc[0]  = _mm_loadu_si128((__m128i*)key);
    enc[1]  = _mm_loadu_si128((__m128i*)(key + 16));
    enc[2]  = expand256_step1(enc[0],  enc[1],  0x01);
    enc[3]  = expand256_step2(enc[2],  enc[1]);
    enc[4]  = expand256_step1(enc[2],  enc[3],  0x02);
    enc[5]  = expand256_step2(enc[4],  enc[3]);
    enc[6]  = expand256_step1(enc[4],  enc[5],  0x04);
    enc[7]  = expand256_step2(enc[6],  enc[5]);
    enc[8]  = expand256_step1(enc[6],  enc[7],  0x08);
    enc[9]  = expand256_step2(enc[8],  enc[7]);
    enc[10] = expand256_step1(enc[8],  enc[9],  0x10);
    enc[11] = expand256_step2(enc[10], enc[9]);
    enc[12] = expand256_step1(enc[10], enc[11], 0x20);
    enc[13] = expand256_step2(enc[12], enc[11]);
    enc[14] = expand256_step1(enc[12], enc[13], 0x40);
}


int aes_ni_key_expansion(const uint8_t key[], int key_length_bytes, aes_ni_schedule_t *sched) {
    if (key_length_bytes == 32) {
        sched->nr = 14;
        expand_256(key, sched->enc);
    } else {
        fprintf(stderr, "AES-NI : seulement AES-256 (32 octets) supporte.\n");
        return -1;
    }
    int nr = sched->nr;
    sched->dec[0]  = sched->enc[nr];
    for (int i = 1; i < nr; i++)
        sched->dec[i] = _mm_aesimc_si128(sched->enc[nr - i]);
    sched->dec[nr] = sched->enc[0];
    return 0;
}


void aes_ni_encrypt_block(const uint8_t plain[16], uint8_t cipher[16], const aes_ni_schedule_t *sched) {
    __m128i b = _mm_loadu_si128((__m128i*)plain);
    int nr = sched->nr;

    b = _mm_xor_si128(b, sched->enc[0]);
    for (int i = 1; i < nr; i++)
        b = _mm_aesenc_si128(b, sched->enc[i]);
    b = _mm_aesenclast_si128(b, sched->enc[nr]);

    _mm_storeu_si128((__m128i*)cipher, b);
}


void aes_ni_decrypt_block(const uint8_t cipher[16], uint8_t plain[16], const aes_ni_schedule_t *sched) {
    __m128i b = _mm_loadu_si128((__m128i*)cipher);
    int nr = sched->nr;

    b = _mm_xor_si128(b, sched->dec[0]);
    for (int i = 1; i < nr; i++)
        b = _mm_aesdec_si128(b, sched->dec[i]);
    b = _mm_aesdeclast_si128(b, sched->dec[nr]);

    _mm_storeu_si128((__m128i*)plain, b);
}


// Chiffre 8 blocs en meme temps pour cacher la latence de 4 cycles de _mm_aesenc_si128
static void encrypt_8blocs(unsigned char *data, const aes_ni_schedule_t *sched) {
    int nr = sched->nr;
    __m128i b0 = _mm_loadu_si128((__m128i*)(data +   0));
    __m128i b1 = _mm_loadu_si128((__m128i*)(data +  16));
    __m128i b2 = _mm_loadu_si128((__m128i*)(data +  32));
    __m128i b3 = _mm_loadu_si128((__m128i*)(data +  48));
    __m128i b4 = _mm_loadu_si128((__m128i*)(data +  64));
    __m128i b5 = _mm_loadu_si128((__m128i*)(data +  80));
    __m128i b6 = _mm_loadu_si128((__m128i*)(data +  96));
    __m128i b7 = _mm_loadu_si128((__m128i*)(data + 112));

    b0 = _mm_xor_si128(b0, sched->enc[0]);
    b1 = _mm_xor_si128(b1, sched->enc[0]);
    b2 = _mm_xor_si128(b2, sched->enc[0]);
    b3 = _mm_xor_si128(b3, sched->enc[0]);
    b4 = _mm_xor_si128(b4, sched->enc[0]);
    b5 = _mm_xor_si128(b5, sched->enc[0]);
    b6 = _mm_xor_si128(b6, sched->enc[0]);
    b7 = _mm_xor_si128(b7, sched->enc[0]);

    for (int r = 1; r < nr; r++) {
        b0 = _mm_aesenc_si128(b0, sched->enc[r]);
        b1 = _mm_aesenc_si128(b1, sched->enc[r]);
        b2 = _mm_aesenc_si128(b2, sched->enc[r]);
        b3 = _mm_aesenc_si128(b3, sched->enc[r]);
        b4 = _mm_aesenc_si128(b4, sched->enc[r]);
        b5 = _mm_aesenc_si128(b5, sched->enc[r]);
        b6 = _mm_aesenc_si128(b6, sched->enc[r]);
        b7 = _mm_aesenc_si128(b7, sched->enc[r]);
    }

    b0 = _mm_aesenclast_si128(b0, sched->enc[nr]);
    b1 = _mm_aesenclast_si128(b1, sched->enc[nr]);
    b2 = _mm_aesenclast_si128(b2, sched->enc[nr]);
    b3 = _mm_aesenclast_si128(b3, sched->enc[nr]);
    b4 = _mm_aesenclast_si128(b4, sched->enc[nr]);
    b5 = _mm_aesenclast_si128(b5, sched->enc[nr]);
    b6 = _mm_aesenclast_si128(b6, sched->enc[nr]);
    b7 = _mm_aesenclast_si128(b7, sched->enc[nr]);

    _mm_storeu_si128((__m128i*)(data +   0), b0);
    _mm_storeu_si128((__m128i*)(data +  16), b1);
    _mm_storeu_si128((__m128i*)(data +  32), b2);
    _mm_storeu_si128((__m128i*)(data +  48), b3);
    _mm_storeu_si128((__m128i*)(data +  64), b4);
    _mm_storeu_si128((__m128i*)(data +  80), b5);
    _mm_storeu_si128((__m128i*)(data +  96), b6);
    _mm_storeu_si128((__m128i*)(data + 112), b7);
}

static void decrypt_8blocs(unsigned char *data, const aes_ni_schedule_t *sched) {
    int nr = sched->nr;
    __m128i b0 = _mm_loadu_si128((__m128i*)(data +   0));
    __m128i b1 = _mm_loadu_si128((__m128i*)(data +  16));
    __m128i b2 = _mm_loadu_si128((__m128i*)(data +  32));
    __m128i b3 = _mm_loadu_si128((__m128i*)(data +  48));
    __m128i b4 = _mm_loadu_si128((__m128i*)(data +  64));
    __m128i b5 = _mm_loadu_si128((__m128i*)(data +  80));
    __m128i b6 = _mm_loadu_si128((__m128i*)(data +  96));
    __m128i b7 = _mm_loadu_si128((__m128i*)(data + 112));

    b0 = _mm_xor_si128(b0, sched->dec[0]);
    b1 = _mm_xor_si128(b1, sched->dec[0]);
    b2 = _mm_xor_si128(b2, sched->dec[0]);
    b3 = _mm_xor_si128(b3, sched->dec[0]);
    b4 = _mm_xor_si128(b4, sched->dec[0]);
    b5 = _mm_xor_si128(b5, sched->dec[0]);
    b6 = _mm_xor_si128(b6, sched->dec[0]);
    b7 = _mm_xor_si128(b7, sched->dec[0]);

    for (int r = 1; r < nr; r++) {
        b0 = _mm_aesdec_si128(b0, sched->dec[r]);
        b1 = _mm_aesdec_si128(b1, sched->dec[r]);
        b2 = _mm_aesdec_si128(b2, sched->dec[r]);
        b3 = _mm_aesdec_si128(b3, sched->dec[r]);
        b4 = _mm_aesdec_si128(b4, sched->dec[r]);
        b5 = _mm_aesdec_si128(b5, sched->dec[r]);
        b6 = _mm_aesdec_si128(b6, sched->dec[r]);
        b7 = _mm_aesdec_si128(b7, sched->dec[r]);
    }

    b0 = _mm_aesdeclast_si128(b0, sched->dec[nr]);
    b1 = _mm_aesdeclast_si128(b1, sched->dec[nr]);
    b2 = _mm_aesdeclast_si128(b2, sched->dec[nr]);
    b3 = _mm_aesdeclast_si128(b3, sched->dec[nr]);
    b4 = _mm_aesdeclast_si128(b4, sched->dec[nr]);
    b5 = _mm_aesdeclast_si128(b5, sched->dec[nr]);
    b6 = _mm_aesdeclast_si128(b6, sched->dec[nr]);
    b7 = _mm_aesdeclast_si128(b7, sched->dec[nr]);

    _mm_storeu_si128((__m128i*)(data +   0), b0);
    _mm_storeu_si128((__m128i*)(data +  16), b1);
    _mm_storeu_si128((__m128i*)(data +  32), b2);
    _mm_storeu_si128((__m128i*)(data +  48), b3);
    _mm_storeu_si128((__m128i*)(data +  64), b4);
    _mm_storeu_si128((__m128i*)(data +  80), b5);
    _mm_storeu_si128((__m128i*)(data +  96), b6);
    _mm_storeu_si128((__m128i*)(data + 112), b7);
}

int aes_ni_ecb_encrypt(unsigned char **dst, int *dst_len,
                       const unsigned char *src, int src_len,
                       const uint8_t key[], int key_length_bytes) {
    int bourrage = 16 - (src_len % 16);
    int total = src_len + bourrage;
    *dst_len = total;
    *dst = (unsigned char*)malloc(total);
    if (!*dst) return -1;
    memcpy(*dst, src, src_len);
    for (int i = 0; i < bourrage; i++)
        (*dst)[src_len + i] = (unsigned char)bourrage;

    aes_ni_schedule_t sched;
    if (aes_ni_key_expansion(key, key_length_bytes, &sched) != 0) { free(*dst); return -1; }

    // Boucle principale : 8 blocs (128 octets) par iteration
    int total8 = (total / 128) * 128;
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < total8; i += 128)
        encrypt_8blocs(*dst + i, &sched);
    // Blocs restants (< 8)
    for (int i = total8; i < total; i += 16)
        aes_ni_encrypt_block(*dst + i, *dst + i, &sched);

    return 0;
}

int aes_ni_ecb_decrypt(unsigned char **dst, int *dst_len,
                       const unsigned char *src, int src_len,
                       const uint8_t key[], int key_length_bytes) {
    if (src_len % 16 != 0) return -1;
    *dst = (unsigned char*)malloc(src_len);
    if (!*dst) return -1;
    memcpy(*dst, src, src_len);

    aes_ni_schedule_t sched;
    if (aes_ni_key_expansion(key, key_length_bytes, &sched) != 0) { free(*dst); return -1; }

    int src8 = (src_len / 128) * 128;
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < src8; i += 128)
        decrypt_8blocs(*dst + i, &sched);
    for (int i = src8; i < src_len; i += 16)
        aes_ni_decrypt_block(*dst + i, *dst + i, &sched);

    int bourrage = (*dst)[src_len - 1];
    if (bourrage <= 0 || bourrage > 16) return -1;
    *dst_len = src_len - bourrage;
    return 0;
}
