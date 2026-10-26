#include "fichier.h"
#include "fonctions.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>




int aes_ecb_encrypt(unsigned char** dst, int *dst_data_length, const unsigned char* src,
int src_data_length, const uint8_t key[], int key_length){
    int nb_de_block = src_data_length / 16;
    if (src_data_length % 16 != 0) {
        nb_de_block++;
    }
    int bourrage = 16 - (src_data_length % 16);
    int total_length = src_data_length + bourrage;
    *dst_data_length = total_length;
    *dst = (unsigned char*)malloc(total_length);
    if (*dst == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire.\n");
        return -1;
    }
    memcpy(*dst, src, src_data_length);
    for (int i = 0; i < bourrage; ++i) {
        (*dst)[src_data_length + i] = (unsigned char)bourrage;
    }
    uint8_t clee_etendue[240];
    keyExpansion(key, nkValue(key_length), key_length, clee_etendue);
    #pragma omp parallel for
    for (int i = 0; i < total_length; i += 16) {
        state_t state_base_local;
        state_t state_code_local;
        chaine_to_state(state_base_local, *dst + i);
        aes_encrypt_avec_clee_etendue(state_code_local, state_base_local, clee_etendue, key_length);
        state_to_chain(*dst + i, state_code_local);
    }
    return 0;
}



int aes_ecb_decrypt(unsigned char** dst, int *dst_data_length, const unsigned char* src,
int src_data_length, const uint8_t key[], int key_length){
    if (src_data_length % 16 != 0) {
        fprintf(stderr, "Erreur : la taille des données chiffrées doit être un multiple de 16 octets.\n");
        return -1;
    }
    *dst = (unsigned char*)malloc(src_data_length);
    if (*dst == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire.\n");
        return -1;
    }
    memcpy(*dst, src, src_data_length);
    uint8_t clee_etendue[240];
    keyExpansion(key, nkValue(key_length), key_length, clee_etendue);
    #pragma omp parallel for
    for (int i = 0; i < src_data_length; i += 16) {
        state_t state_a_dechiffrer_local;
        state_t state_dechiffre_local;
        chaine_to_state(state_a_dechiffrer_local, *dst + i);
        aes_decrypt_avec_clee_etendue(state_dechiffre_local, state_a_dechiffrer_local, clee_etendue, key_length);
        state_to_chain(*dst + i, state_dechiffre_local);
    }
    int bourrage = (*dst)[src_data_length - 1];
    if (bourrage <= 0 || bourrage > 16) {
        fprintf(stderr, "Erreur : valeur de bourrage invalide.\n");
         *dst_data_length = src_data_length;
        return -1;
    }

    *dst_data_length = src_data_length - bourrage;
    return 0;
}

int aes_cbc_encrypt(unsigned char** dst, int *dst_data_length, const unsigned char* src,
                    int src_data_length, const unsigned char* iv, const uint8_t key[], int key_length) {
    state_t state_base;
    state_t state_code;
    int nb_de_block = src_data_length / 16;
    if (src_data_length % 16 != 0) {
        nb_de_block++;
    }
    int bourrage = 16 - (src_data_length % 16);
    int total_length = src_data_length + bourrage;
    *dst_data_length = total_length;
    *dst = (unsigned char*)malloc(total_length);
    if (*dst == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire.\n");
        return -1;
    }
    memcpy(*dst, src, src_data_length);
    for (int i = 0; i < bourrage; ++i) {
        (*dst)[src_data_length + i] = (unsigned char)bourrage;
    }
    uint8_t clee_etendue[240];
    keyExpansion(key, nkValue(key_length), key_length, clee_etendue);
    const unsigned char* prev_block = iv;

    for (int i = 0; i < total_length; i += 16) {
        for (int j = 0; j < 16; j++) {
            (*dst)[i + j] ^= prev_block[j];
        }
        chaine_to_state(state_base, *dst + i);
        aes_encrypt_avec_clee_etendue(state_code, state_base, clee_etendue, key_length);
        state_to_chain(*dst + i, state_code);
        prev_block = *dst + i;
    }
    return 0;
}

int aes_cbc_decrypt(unsigned char** dst, int *dst_data_length, const unsigned char* src,
                    int src_data_length, const unsigned char* iv, const uint8_t key[], int key_length) {
    if (src_data_length % 16 != 0) {
        fprintf(stderr, "Erreur : la taille des données chiffrées doit être un multiple de 16 octets.\n");
        return -1;
    }
    *dst = (unsigned char*)malloc(src_data_length);
    if (*dst == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire.\n");
        return -1;
    }

    uint8_t clee_etendue[240];
    keyExpansion(key, nkValue(key_length), key_length, clee_etendue);

    #pragma omp parallel for
    for (int i = 0; i < src_data_length; i += 16) {
        state_t state_a_dechiffrer_local;
        state_t state_dechiffre_local;
        chaine_to_state(state_a_dechiffrer_local, src + i);
        aes_decrypt_avec_clee_etendue(state_dechiffre_local, state_a_dechiffrer_local, clee_etendue, key_length);
        state_to_chain(*dst + i, state_dechiffre_local);
        // XOR avec le bloc chiffre precedent (ou IV pour le premier bloc)
        const unsigned char* prev = (i == 0) ? iv : src + i - 16;
        for (int j = 0; j < 16; j++) {
            (*dst)[i + j] ^= prev[j];
        }
    }
    
    int bourrage = (*dst)[src_data_length - 1];
    if (bourrage <= 0 || bourrage > 16) {
        fprintf(stderr, "Erreur : valeur de bourrage invalide.\n");
         *dst_data_length = src_data_length;
        return -1;
    }
    
    *dst_data_length = src_data_length - bourrage;
    return 0;
}
int aes_fichier(char clee_hex[], int longueur_clee, int mode_cbc, unsigned char iv[16])
{
    uint8_t clee[32];
    cle_hex_vers_tableau(clee_hex, clee);
    char fichier_entree[256];
    char fichier_sortie[256];
    printf("Nom du fichier d'entrée : ");
    if (scanf("%255s", fichier_entree) != 1) { fprintf(stderr, "Saisie invalide.\n"); return -1; }
    printf("Nom du fichier de sortie : ");
    if (scanf("%255s", fichier_sortie) != 1) { fprintf(stderr, "Saisie invalide.\n"); return -1; }
    FILE* fichier = NULL;
    FILE* fichier_s  = NULL;
    long length = 0;      
    fichier = fopen(fichier_entree, "rb");
    if (fichier == NULL) {
        perror("Erreur lors de l'ouverture du fichier source");
        return -1;
    }
    fichier_s = fopen(fichier_sortie, "wb");
    if (fichier_s == NULL) {
        perror("Erreur lors de l'ouverture du fichier de sortie");
        fclose(fichier);
        return -1;
    }
    fseek(fichier, 0, SEEK_END);
    length = ftell(fichier);
    rewind(fichier);
    unsigned char* message_a_chiffrer = (unsigned char*)malloc(length);
    if (message_a_chiffrer == NULL) {
        fprintf(stderr, "Erreur d'allocation.\n");
        fclose(fichier);
        fclose(fichier_s);
        return -1;
    }
    int octet_lu = (int)fread(message_a_chiffrer, 1, length, fichier);
    unsigned char* message_chiffre = NULL;
    int longueur_message_chiffre;
    int mode;
    printf("Mode (1 = chiffrer, 2 = déchiffrer) : ");
    if (scanf("%d", &mode) != 1) mode = 0;
    clock_t debut = clock();
    if (mode == 1) {
        if (mode_cbc) {
            aes_cbc_encrypt(&message_chiffre, &longueur_message_chiffre, message_a_chiffrer, octet_lu, iv, clee, calculNr(longueur_clee));
        } else {
            aes_ecb_encrypt(&message_chiffre, &longueur_message_chiffre, message_a_chiffrer, octet_lu, clee, calculNr(longueur_clee));
        }
    } else if (mode == 2) {
        if (mode_cbc) {
            aes_cbc_decrypt(&message_chiffre, &longueur_message_chiffre, message_a_chiffrer, octet_lu, iv, clee, calculNr(longueur_clee));
        } else {
            aes_ecb_decrypt(&message_chiffre, &longueur_message_chiffre, message_a_chiffrer, octet_lu, clee, calculNr(longueur_clee));
        }
    } else {
        fprintf(stderr, "Mode invalide.\n");
        free(message_a_chiffrer);
        fclose(fichier);
        fclose(fichier_s);
        return -1;
    }
    clock_t fin = clock();
    double duree_ms = (double)(fin - debut) / CLOCKS_PER_SEC * 1000.0;
    double debit_mbs = (double)octet_lu / (1024.0 * 1024.0) / ((double)(fin - debut) / CLOCKS_PER_SEC);
    printf("Temps : %.3f ms | Débit : %.2f Mo/s | %d octets traités\n",
           duree_ms, debit_mbs, octet_lu);

    fwrite(message_chiffre, 1, longueur_message_chiffre, fichier_s);
    free(message_a_chiffrer);
    free(message_chiffre);
    fclose(fichier);
    fclose(fichier_s);  


    return 0;
}