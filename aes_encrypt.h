#ifndef AES_ENCRYPT_H
#define AES_ENCRYPT_H

#include "fonctions.h"

void sub_bytes(state_t state);
void shift_rows(state_t state);
void mix_columns(state_t state);

#endif