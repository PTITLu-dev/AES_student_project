# 🔒 AES Encryption Engine & Analysis Tool

![C](https://img.shields.io/badge/Language-C-blue.svg)
![OpenMP](https://img.shields.io/badge/Parallel-OpenMP-orange.svg)
![AES-NI](https://img.shields.io/badge/Hardware-AES--NI-red.svg)

## 📝 Description
Ce projet est une implémentation du standard de chiffrement **AES (Advanced Encryption Standard)** en langage C. Réalisé en binôme dans le cadre de ma deuxième année de cycle préparatoire à l'**Esisar**, il explore l'approche logicielle classique, l'accélération matérielle et la parallélisation.

L'outil permet de chiffrer/déchiffrer des chaînes de caractères, des fichiers bruts, ainsi que des images BMP (avec préservation de l'en-tête pour visualiser concrètement l'effet du chiffrement sur les pixels).

## ✨ Fonctionnalités implémentées
* **Chiffrement Logiciel de A à Z :** Implémentation mathématique complète (SubBytes, ShiftRows, MixColumns, AddRoundKey).
* **Accélération Matérielle (AES-NI) :** Utilisation des instructions intrinsèques SIMD d'Intel pour des performances maximales.
* **Parallélisation (OpenMP) :** Traitement des blocs indépendants en parallèle pour optimiser l'utilisation du processeur multicœur.
* **Modes d'opération :** Electronic Codebook (ECB) et Cipher Block Chaining (CBC).
* **Analyse Statistique :** Calcul de l'entropie de Shannon sur les images (pour comparer visuellement et mathématiquement les modes ECB et CBC).

## ⚠️ Regard critique et Limites connues (Axes d'amélioration)
Ce projet étant un livrable académique avec une contrainte de temps, certains choix d'architecture ont été faits pour privilégier la compréhension de l'algorithme plutôt que la robustesse pour un environnement de production. J'ai parfaitement conscience des limites suivantes :

1. **Gestion de la mémoire sur les gros fichiers :**
   * *État actuel :* Le programme lit l'intégralité du fichier cible d'un seul coup et le charge dans la RAM (`malloc` de la taille du fichier).
   * *Conséquence :* Cela fonctionne très bien pour des fichiers légers ou moyens, mais provoquera un crash (saturation mémoire) sur des fichiers de plusieurs gigaoctets.
   * *Solution idéale :* Traiter les fichiers par flux (streams) en lisant et chiffrant des morceaux (chunks) de 4 Ko par itération.

2. **Sécurité du mode CBC (Vecteur d'Initialisation statique) :**
   * *État actuel :* Le Vecteur d'Initialisation (IV) du mode CBC est codé en dur (tableau de zéros).
   * *Conséquence :* Chiffrer deux fois le même fichier avec la même clé produira exactement le même résultat, ce qui rend le chiffrement vulnérable à des attaques par analyse de trafic.
   * *Solution idéale :* Utiliser un générateur de nombres pseudo-aléatoires sécurisé (`/dev/urandom`) pour créer un nouvel IV à chaque session, et concaténer cet IV au tout début du fichier chiffré.

3. **Interface Utilisateur (CLI) :**
   * *État actuel :* L'interface repose sur un menu interactif avec des `scanf`.
   * *Solution idéale :* Une vraie gestion des arguments en ligne de commande via `argc`/`argv` (ex: `./aes --encrypt --mode cbc --in data.txt`).

## 🛠️ Compilation et Exécution

**Prérequis :** Linux, `gcc` avec OpenMP, `make`, et un processeur x86-64 avec les instructions AES-NI (option 5 et images en ECB).

```bash
make            # construit ./aes (outil interactif) et ./test_aes
make test       # vérifie le chiffrement sur les vecteurs officiels FIPS-197
./aes           # menu : bloc, fichier, texte de démonstration, image BMP, test de performance
make clean
```

La clé utilisée par l'outil est la clé de test AES-256 du standard FIPS-197
(`000102…1e1f`), codée en dur dans `main.c` pour la démonstration.

## 📂 Structure du code

- `aes_encrypt.c` / `aes_decrypt.c` : AES logiciel (SubBytes, ShiftRows, MixColumns, AddRoundKey)
- `fonctions.c` : outils communs (arithmétique dans GF(2⁸), expansion de clé)
- `aes_ni.c` : version accélérée avec les instructions AES-NI (`wmmintrin.h`) et OpenMP
- `fichier.c` : chiffrement de fichiers, modes ECB et CBC, bourrage PKCS#7
- `photo.c` / `bmp.c` : chiffrement des pixels d'une image BMP et calcul de l'entropie
- `main.c` : menu interactif ; `test.c` : tests sur les vecteurs FIPS-197

## 👥 Auteurs
- PETIT Lucas
- YOUSSEF Maïssa
