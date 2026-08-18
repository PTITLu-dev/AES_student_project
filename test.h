#ifndef TEST_H
#define TEST_H

#include "fonctions.h"

void test_sub_bytes(state_t state);
void test_shift_rows(state_t state);
void test_mix_columns(state_t state);

void test_inv_sub_bytes(state_t state);
void test_inv_shift_rows(state_t state);
void test_inv_mix_columns(state_t state);
void test_aes_encrypt(state_t etat_entree, state_t etat_sortie);
void test_aes_decrypt(state_t etat_entree, state_t etat_sortie);

#endif