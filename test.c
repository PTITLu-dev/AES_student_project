#include <stdio.h>
#include "test.h"
#include "fonctions.h"
#include "aes_encrypt.h"
#include "aes_decrypt.h"


void test_sub_bytes(state_t state) {
    // Entrée du test FIPS
    texte_hex_vers_state("193DE3BEA0F4E22B9AC68D2AE9F84808", state);
    print_state(state);
    sub_bytes(state);
    print_state_ligne(state);
    //ATTENDU : d42711aee0bf98f1b8b45de51e415230
}

void test_shift_rows(state_t state) {
    texte_hex_vers_state("D42711AEE0BF98F1B8B45DE51E415230", state);
    print_state(state);
    shift_rows(state);
    printf("Sortie obtenue : ");
    print_state_ligne(state);
    //ATTENDU: d4bf5d30e0b452aeb84111f11e2798e5
}

void test_mix_columns(state_t state) {    
    texte_hex_vers_state("D4BF5D30E0B452AEB84111F11E2798E5", state);
    print_state(state);
    mix_columns(state);
    printf("Sortie obtenue : ");
    print_state_ligne(state);
    //ATTENDU : 046681e5e0cb199a48f8d37a2806264c
}



void test_inv_sub_bytes(state_t state) {
    texte_hex_vers_state("D42711AEE0BF98F1B8B45DE51E415230", state);
    inv_sub_bytes(state);
    printf("Sortie obtenue : ");
    print_state_ligne(state);
    //ATTENDU: 193de3bea0f4e22b9ac68d2ae9f84808
}

void test_inv_shift_rows(state_t state) {
    texte_hex_vers_state("D4BF5D30E0B452AEB84111F11E2798E5", state);
    inv_shift_rows(state);
    printf("Sortie obtenue : ");
    print_state_ligne(state);
    //ATTENDU        : d42711aee0bf98f1b8b45de51e415230
}

void test_inv_mix_columns(state_t state) {
    texte_hex_vers_state("046681E5E0CB199A48F8D37A2806264C", state);
    inv_mix_columns(state);
    printf("Sortie obtenue : ");
    print_state_ligne(state);
    //ATTENDU : d4bf5d30e0b452aeb84111f11e2798e5
}



void test_aes_encrypt(state_t etat_entree, state_t etat_sortie) {
    texte_hex_vers_state("00112233445566778899AABBCCDDEEFF", etat_entree);
    
    // Clé de chiffrement (16 octets)
    uint8_t cle[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 
                       0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    
    aes_encrypt(etat_sortie, etat_entree, cle, 10);
    printf("Sortie obtenue : ");
    print_state_ligne(etat_sortie);
    //ATTENDU: 69c4e0d86a7b0430d8cdb78070b4c55a
}

void test_aes_decrypt(state_t etat_entree, state_t etat_sortie) {
    texte_hex_vers_state("69C4E0D86A7B0430D8CDB78070B4C55A", etat_entree);
    // Clé de chiffrement 
    uint8_t cle[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 
                       0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    
    aes_decrypt(etat_sortie, etat_entree, cle, 10);
    printf("Sortie obtenue : ");
    print_state_ligne(etat_sortie);
    printf("ATTENDU        : 00112233445566778899aabbccddeeff\n\n");
}


int main() {
    state_t etat_test;
    state_t etat_entree;
    state_t etat_sortie;

    test_sub_bytes(etat_test);
    test_shift_rows(etat_test);
    test_mix_columns(etat_test);

    test_inv_sub_bytes(etat_test);
    test_inv_shift_rows(etat_test);
    test_inv_mix_columns(etat_test);

    test_aes_encrypt(etat_entree, etat_sortie);
    test_aes_decrypt(etat_entree, etat_sortie);


    return 0;
}