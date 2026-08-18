/* bmp.h: utilitaires pour la manipulation de fichiers images au format BMP */

/* Types de données. */

/* Image BMP en mémoire */
typedef struct bmp_image bmp_image;

/* Bloc de mémoire,l embarquant l'information de sa taille */
typedef struct chunk chunk;

/* Concaténation de deux chunks */
chunk*
chunk_concat(chunk *a, chunk *b);

/* Libération d'un bloc */
void
chunk_free(chunk *ch);

/* Copie d'un bloc */
chunk*
chunk_copy(chunk *src);

/* Création d'un bloc à partir d'un pointeur et d'une taille */
chunk*
as_chunk(char *src, int size);

/* Taille d'un bloc */
int
chunk_get_size(chunk *c);

/* (Pointeur vers) les données d'un bloc */
char*
chunk_get_data(chunk *c);

/* Chargement d'une image BMP vers la mémoire */
bmp_image*
bmp_read(char *fname);

/* Enregistrement d'une image BMP sur disque */
void
bmp_write(char *fname, bmp_image *img);

/* Libération de la mémoire d'une image BMP */
void
bmp_free(bmp_image *img);

/* Création d'une image à partir d'un bloc d'en-tête et d'un bloc de pixels */
bmp_image*
bmp_create(chunk *header, chunk *pixels);

/* Récupération de l'en-tête d'une image */
chunk*
bmp_get_header(bmp_image *img);

/* Récupération des pixels d'une image */
chunk*
bmp_get_pixels(bmp_image *img);
