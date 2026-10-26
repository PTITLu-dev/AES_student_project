#include "fonctions.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

const uint8_t Rcon[11] = {
    0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36
};


void print_state(state_t state){
    printf("\n");
    for(int i=0;i<4;i++){
        for(int j=0;j<4;j++){
            printf("%02x ", state[i][j]);
        }
        printf("\n");
    }
printf("\n");
}

// Affiche le state sous forme d'une seule ligne hexa, dans l'ordre de sortie AES
// (colonne par colonne : state[0][0] state[1][0] state[2][0] state[3][0] state[0][1] ...)
void print_state_ligne(state_t state){
    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 4; i++) {
            printf("%02x", state[i][j]);
        }
    }
    printf("\n");
}



int nkValue(int nr){
    if (nr ==10) {return 4;}
    if (nr==12)  {return 6;}
    if(nr==14)   {return 8;}
    else fprintf(stderr, "Nombre de tours invalide.\n");
    return 0;
}


int calculNr(int key_length) {
    if (key_length == 16) {
        return 10; 
    } else if (key_length == 24) {
        return 12; 
    } else if (key_length == 32) {
        return 14; 
    } else {
        fprintf(stderr, "Taille de clé invalide : 16, 24 ou 32 octets attendus.\n");
        return -1; 
    }
}
void add_round_key(state_t state, const uint8_t* round_key){
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            state[i][j] ^= round_key[j * 4 + i];
        }
    }
}


void RotWord(uint8_t word[4]) {
    uint8_t temp = word[0];
    word[0] = word[1];
    word[1] = word[2];
    word[2] = word[3];
    word[3] = temp;
}

void SubWord(uint8_t word[4]) {
    for (int i = 0; i < 4; i++) {
        word[i] = sbox[word[i]];
    }
}




void keyExpansion(const uint8_t *key, int nk, int nr, uint8_t *expanded_key) {
    int total_words = 4 * (nr + 1); 
    uint8_t temp_word[4];
    int i=0;
    for (i = 0; i < nk * 4; i++) {
        expanded_key[i] = key[i];
    }
    for (i = nk; i < total_words; i++) {
        for (int j = 0; j < 4; j++) {
            temp_word[j] = expanded_key[4 * (i - 1) + j];
        }
        if (i % nk == 0) {
            RotWord(temp_word);
            SubWord(temp_word);
            temp_word[0] ^= Rcon[i / nk];
        } else if (nk > 6 && (i % nk == 4)) {
            SubWord(temp_word);
        }
        for (int j = 0; j < 4; j++) {
            expanded_key[4 * i + j] = expanded_key[4 * (i - nk) + j] ^ temp_word[j];
        }
    }

}

//foncyion de debug
void print_expanded_key(const uint8_t *expanded_key, int nr) {
    int total_words = 4 * (nr + 1);
    printf("Cle etendue (mots uniquement):\n");
    for (int word_idx = 0; word_idx < total_words; word_idx++) {
        printf("  Mot %d: ", word_idx);
        printf("\"");
        const uint8_t *current_word_start = &expanded_key[word_idx * 4];
        for (int byte_idx = 0; byte_idx < 4; byte_idx++) {
            printf("%02x", current_word_start[byte_idx]);
        }
        printf("\"\n");
    }
}



uint8_t mult(uint8_t a, int par) {
    uint8_t res_a_mult_2;   
    uint8_t res_a_mult_4;   
    uint8_t res_a_mult_8;
    res_a_mult_2 = (a << 1) ^ ((a & 0x80) ? 0x1b : 0);
    res_a_mult_4 = (res_a_mult_2 << 1) ^ ((res_a_mult_2 & 0x80) ? 0x1b : 0);
    res_a_mult_8 = (res_a_mult_4 << 1) ^ ((res_a_mult_4 & 0x80) ? 0x1b : 0);
    if (par == 2) {
        return res_a_mult_2;
    }
    if (par == 3) {
        return res_a_mult_2 ^ a;
    }
    if (par == 9) {
        //multiplier par 9 = 8 +1 = 2*2*2+1
        return res_a_mult_8 ^ a;
    }
    if (par == 11) {
        //multiplier par 11 = 8 +2 +1 = 2*2*2+2+1
        return (res_a_mult_8 ^ res_a_mult_2) ^ a;
    }
    if (par == 13) {
        //multiplier par 13 = 8 +4 +1 = 2*2*2+4+1
        return (res_a_mult_8 ^ res_a_mult_4) ^ a;
    }
    if (par == 14) {
        //multiplier par 14 = 8 +4 +2 = 2*2*2+4+2
       return (res_a_mult_8 ^ res_a_mult_4 ^ res_a_mult_2);
    }
    return 0;
}



void tester_expansion(const uint8_t *key, int key_length_bytes) {
    int nr = calculNr(key_length_bytes);
    int nk = nkValue(nr); 
    
    uint8_t expanded_key[4 * (nr + 1) * 4]; 

    printf("=== Test pour une cle de %d bits ===\n", key_length_bytes * 8);
    keyExpansion(key, nk, nr, expanded_key);
    print_expanded_key(expanded_key, nr);
    printf("\n");
}


static uint8_t hex_char_vers_valeur(char c) {
    if (c >= '0' && c <= '9') return (uint8_t)(c - '0');
    if (c >= 'a' && c <= 'f') return (uint8_t)(c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return (uint8_t)(c - 'A' + 10);
    return 0;
}

void hex_vers_octets(const char *hex, uint8_t *octets, int nb_octets) {
    for (int i = 0; i < nb_octets; i++) {
        octets[i] = (uint8_t)((hex_char_vers_valeur(hex[2 * i]) << 4) | hex_char_vers_valeur(hex[2 * i + 1]));
    }
}

void cle_hex_vers_tableau(const char *hex, uint8_t *cle) {
    hex_vers_octets(hex, cle, (int)(strlen(hex) / 2));
}

void texte_hex_vers_state(const char *hex, state_t state) {
    uint8_t octets[16];
    hex_vers_octets(hex, octets, 16);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            state[j][i] = octets[4 * i + j];
        }
    }
}

// Equivalent de texte_hex_vers_state mais a partir de 16 octets bruts (pas une chaine hexa)
void chaine_to_state(state_t state, const unsigned char *octets) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            state[j][i] = octets[4 * i + j];
        }
    }
}

// Operation inverse de chaine_to_state : recopie le state dans 16 octets bruts
void state_to_chain(unsigned char *octets, state_t state) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            octets[4 * i + j] = state[j][i];
        }
    }
}



