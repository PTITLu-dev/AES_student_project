#include "aes.h"
#include "aes_encrypt.h"
#include "fonctions.h"
#include "fichier.h"
#include "photo.h"
#include "aes_ni.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>



int main_chaine( char clee_hex[], int taille_cle){ 
    state state_utilisateur;
    state state_code;
    uint8_t clee[32];
    printf("Entrez un bloc de 16 octets en hexadécimal (32 caractères) :\n");
    char texte_hex[33];
    if (scanf("%32s", texte_hex) != 1) { fprintf(stderr, "Saisie invalide.\n"); return 1; }
    texte_hex_vers_state(texte_hex, state_utilisateur);
    cle_hex_vers_tableau(clee_hex, clee);
    printf("Chiffrer (1) ou déchiffrer (2) ?\n");
    int choix;
    if (scanf("%d", &choix) != 1) { fprintf(stderr, "Saisie invalide.\n"); return 1; }
        if (choix == 1) {
            aes_encrypt(state_code, state_utilisateur, clee, calculNr(taille_cle));
        } else if (choix == 2) {
            aes_decrypt( state_code, state_utilisateur, clee, calculNr(taille_cle));
        } else {
            printf("Choix invalide : 1 ou 2.\n");
            return 1;
        }
        printf("Résultat :\n");
            print_state(state_code);

    return 0;
}


char char_test[46] = "Bonjour, nous sommes le binome 11!";
char char_test_long[446] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";

int main_long_chaine(char clee_hex[], int taille_cle){
    uint8_t clee[32];
    unsigned char *char_resultat = NULL;
    int taille_cle_resultat;
    printf("Texte de démonstration (mode ECB) :\n");
    printf("%s\n", char_test);
    cle_hex_vers_tableau(clee_hex, clee);
    printf("Chiffrer (1) ou déchiffrer (2) ?\n");
    int choix;
    if (scanf("%d", &choix) != 1) { fprintf(stderr, "Saisie invalide.\n"); return 1; }
        if (choix == 1) {
            aes_ecb_encrypt( &char_resultat, &taille_cle_resultat, (unsigned char*)char_test, (int)strlen(char_test), clee, calculNr(taille_cle));
        } else if (choix == 2) {
            unsigned char *char_chiffre = NULL;
            int taille_chiffre;
            aes_ecb_encrypt( &char_chiffre, &taille_chiffre, (unsigned char*)char_test, (int)strlen(char_test), clee, calculNr(taille_cle));
            aes_ecb_decrypt( &char_resultat, &taille_cle_resultat, char_chiffre, taille_chiffre, clee, calculNr(taille_cle));
            free(char_chiffre);
        }
        else {
            printf("Choix invalide : 1 ou 2.\n");
            return 1;
        }
        printf("Résultat (hexadécimal) :\n");
        for (int i = 0; i < taille_cle_resultat; i++) {
            printf("%02x", char_resultat[i]);

        }
        printf("\nRésultat (texte) :\n");
        for (int i = 0; i < taille_cle_resultat; i++) {
            printf("%c", char_resultat[i]);
        }
        printf("\n");
        free(char_resultat);
    return 0;

}




static void test(const char *clee_hex, int taille_clee) {
    printf("Taille du tampon à tester en Mo (ex : 500) : ");
    int taille_mb;
    if (scanf("%d", &taille_mb) != 1 || taille_mb <= 0 || taille_mb > 2047) { fprintf(stderr, "Taille invalide (1 à 2047 Mo).\n"); return; }

    int taille = taille_mb * 1024 * 1024;
    unsigned char *data = (unsigned char*)malloc(taille);
    if (!data) { fprintf(stderr, "Allocation échouée.\n"); return; }

    // Remplissage avec des donnees pseudo-aleatoires
    for (int i = 0; i < taille; i++) data[i] = (unsigned char)(i ^ (i >> 8) ^ 0xA5);

    uint8_t clee[32];
    cle_hex_vers_tableau(clee_hex, clee);

    unsigned char *out = NULL;
    int out_len = 0;

    double t0 = omp_get_wtime();
    aes_ni_ecb_encrypt(&out, &out_len, data, taille, clee, taille_clee);
    double t1 = omp_get_wtime();
    printf("AES-NI+OpenMP ECB : %7.1f ms | %6.1f MB/s\n",
           (t1-t0)*1000.0, (double)taille/(1024.0*1024.0)/(t1-t0));
    free(out); out = NULL;

    free(data);
}

int main() {
    char clee_hex[65]="000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f";
    int taille_clee = (int)(strlen(clee_hex) / 2);
    printf("Que voulez-vous faire ?\n1. Chiffrer/déchiffrer un bloc (hexadécimal)\n2. Chiffrer/déchiffrer un fichier\n3. Chiffrer/déchiffrer un texte de démonstration\n4. Chiffrer/déchiffrer une image BMP\n5. Test de performance (AES-NI + OpenMP)\n");
    int choix;
    if (scanf("%d", &choix) != 1) { fprintf(stderr, "Saisie invalide.\n"); return 1; }
    int mode_cbc = 0;
    if (choix != 5) {
        printf("Utiliser le mode CBC ? (1 = oui, 0 = non, ECB)\n");
        if (scanf("%d", &mode_cbc) != 1) { fprintf(stderr, "Saisie invalide.\n"); return 1; }
    }
    unsigned char iv[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    if (choix == 1) {
         return main_chaine(clee_hex, taille_clee);
    } else if (choix == 2) {
        return aes_fichier(clee_hex, taille_clee, mode_cbc, iv);
    } else if (choix == 3) {
        return main_long_chaine(clee_hex, taille_clee);
    } else if (choix == 4) {
        return photo(clee_hex, taille_clee, mode_cbc, iv);
    } else if (choix == 5) {
        test(clee_hex, taille_clee);
        return 0;
    } else {
        printf("Choix invalide : 1 à 5.\n");
        return 1;
    }
    return 0;
}
