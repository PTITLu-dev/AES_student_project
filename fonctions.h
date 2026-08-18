
#ifndef FONCTIONS_H
#define FONCTIONS_H

#include <stdint.h>
#include <stdio.h>


extern const uint8_t sbox[256];
extern const uint8_t inv_sbox[256];
extern const uint8_t Rcon[11];

typedef uint8_t state_t[4][4];

void print_state(state_t state);
void print_state_ligne(state_t state);
void print_expanded_key(const uint8_t *expanded_key, int nr);
void tester_expansion(const uint8_t *key, int key_length_bytes);
int nkValue(int nr);
int calculNr(int key_length);
void add_round_key(state_t state, const uint8_t* round_key);
uint8_t mult(uint8_t a, int par);
void RotWord(uint8_t word[4]);
void SubWord(uint8_t word[4]);
void keyExpansion(const uint8_t *key, int nk, int nr, uint8_t *expanded_key);
void hex_vers_octets(const char *hex, uint8_t *octets, int nb_octets);
void cle_hex_vers_tableau(const char *hex, uint8_t *cle);
void texte_hex_vers_state(const char *hex, state_t state);
void chaine_to_state(state_t state, const unsigned char *octets);
void state_to_chain(unsigned char *octets, state_t state);

#endif // FONCTIONS_H
