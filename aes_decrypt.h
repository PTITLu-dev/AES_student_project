#ifndef AES_DECRYPT_H
#define AES_DECRYPT_H

#include "aes.h"
#include "fonctions.h"

void inv_sub_bytes(state_t state);
void inv_shift_rows(state_t state);
void inv_mix_columns(state_t state);


#endif