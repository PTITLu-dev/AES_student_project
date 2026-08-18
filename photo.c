
#include "bmp.h"
#include "aes.h"
#include "fichier.h"
#include "aes_ni.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>


double entropie(const unsigned char* data, int taille) {
    int freq[256] = {0};
    for (int i = 0; i < taille; i++) {
        freq[data[i]]++;
    }
    double h = 0.0;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            double p = (double)freq[i] / taille;
            h -= p * log2(p);
        }
    }
    return h;
}


int photo(char clee_hex[], int longueur_clee, int mode_cbc, unsigned char iv[16]) {
    bmp_image *img, *img2;
    chunk *h, *h2, *p, *p2;
    printf("Quelle image voulez vous chiffrer ?\n");
    char image[256];
    scanf("%255s", image);
    printf("Quelle image voulez vous sortir ?\n");
    char image_sortie[256];
    scanf("%255s", image_sortie);
    img = bmp_read(image);
    h = bmp_get_header(img);
    p = bmp_get_pixels(img);
    printf("Entropie image source      : %.4f bits/octet\n",
           entropie((const unsigned char*)chunk_get_data(p), chunk_get_size(p)));
    printf("Veux tu cryper (1) ou decrypter (2) ?\n");
    int mode;
    scanf("%d", &mode);
    if (mode == 1) {
        unsigned char* pixels_chiffres = NULL;
        int taille_pixels_chiffres;
        uint8_t clee[32];
        cle_hex_vers_tableau(clee_hex, clee);

        double debut = omp_get_wtime();
        if (mode_cbc) {
            aes_cbc_encrypt(&pixels_chiffres, &taille_pixels_chiffres, (unsigned char*)chunk_get_data(p), chunk_get_size(p), iv, clee, calculNr(longueur_clee));
        } else {
            aes_ecb_encrypt(&pixels_chiffres, &taille_pixels_chiffres, (unsigned char*)chunk_get_data(p), chunk_get_size(p), clee, calculNr(longueur_clee));
        }
        double fin = omp_get_wtime();
        double duree_ms = (fin - debut) * 1000.0;
        double debit_mbs = (double)chunk_get_size(p) / (1024.0 * 1024.0) / (fin - debut);
        printf("Logiciel+OpenMP %s : %.3f ms | %.2f MB/s\n",
               mode_cbc ? "CBC" : "ECB", duree_ms, debit_mbs);
        free(pixels_chiffres); pixels_chiffres = NULL;

        if (!mode_cbc) {
            debut = omp_get_wtime();
            aes_ni_ecb_encrypt(&pixels_chiffres, &taille_pixels_chiffres, (unsigned char*)chunk_get_data(p), chunk_get_size(p), clee, longueur_clee);
            fin = omp_get_wtime();
            duree_ms = (fin - debut) * 1000.0;
            debit_mbs = (double)chunk_get_size(p) / (1024.0 * 1024.0) / (fin - debut);
            printf("AES-NI+OpenMP   ECB : %.3f ms | %.2f MB/s\n", duree_ms, debit_mbs);
        } else {
            aes_cbc_encrypt(&pixels_chiffres, &taille_pixels_chiffres, (unsigned char*)chunk_get_data(p), chunk_get_size(p), iv, clee, calculNr(longueur_clee));
        }

        printf("Entropie image chiffree %s : %.4f bits/octet\n",
               mode_cbc ? "CBC" : "ECB",
               entropie(pixels_chiffres, taille_pixels_chiffres));
        p2 = as_chunk((char*)pixels_chiffres, taille_pixels_chiffres);
        h2 = chunk_copy(h);
        img2 = bmp_create(h2, p2);
    } else if (mode == 2) {
        unsigned char* pixels_dechiffres = NULL;
        int taille_pixels_dechiffres;
        uint8_t clee[32];
        cle_hex_vers_tableau(clee_hex, clee);

        double debut = omp_get_wtime();
        if (mode_cbc) {
            aes_cbc_decrypt(&pixels_dechiffres, &taille_pixels_dechiffres, (unsigned char*)chunk_get_data(p), chunk_get_size(p), iv, clee, calculNr(longueur_clee));
        } else {
            aes_ecb_decrypt(&pixels_dechiffres, &taille_pixels_dechiffres, (unsigned char*)chunk_get_data(p), chunk_get_size(p), clee, calculNr(longueur_clee));
        }
        double fin = omp_get_wtime();
        double duree_ms = (fin - debut) * 1000.0;
        double debit_mbs = (double)chunk_get_size(p) / (1024.0 * 1024.0) / (fin - debut);
        printf("Logiciel+OpenMP %s : %.3f ms | %.2f MB/s\n",
               mode_cbc ? "CBC" : "ECB", duree_ms, debit_mbs);
        free(pixels_dechiffres); pixels_dechiffres = NULL;

        if (!mode_cbc) {
            debut = omp_get_wtime();
            aes_ni_ecb_decrypt(&pixels_dechiffres, &taille_pixels_dechiffres, (unsigned char*)chunk_get_data(p), chunk_get_size(p), clee, longueur_clee);
            fin = omp_get_wtime();
            duree_ms = (fin - debut) * 1000.0;
            debit_mbs = (double)chunk_get_size(p) / (1024.0 * 1024.0) / (fin - debut);
            printf("AES-NI+OpenMP   ECB : %.3f ms | %.2f MB/s\n", duree_ms, debit_mbs);
        } else {
            aes_cbc_decrypt(&pixels_dechiffres, &taille_pixels_dechiffres, (unsigned char*)chunk_get_data(p), chunk_get_size(p), iv, clee, calculNr(longueur_clee));
        }

        printf("Entropie image dechiffree  : %.4f bits/octet\n",
               entropie(pixels_dechiffres, taille_pixels_dechiffres));
        p2 = as_chunk((char*)pixels_dechiffres, taille_pixels_dechiffres);
        h2 = chunk_copy(h);
        img2 = bmp_create(h2, p2);
    } else {
        printf("heu pas encore code seulement 1 ou 2 !!.\n");
        return 1;
    }
    bmp_free(img);
    bmp_write(image_sortie, img2);
    bmp_free(img2);

  return 0;
}
