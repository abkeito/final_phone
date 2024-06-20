#ifndef CRYPTO_H
#define CRYPTO_H

#include <openssl/evp.h>

#define KEY_LENGTH 32
#define IV_LENGTH 16

int myencrypt(unsigned char *plaintext, int plaintext_len, unsigned char *ciphertext);
int mydecrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *plaintext);

#endif // CRYPTO_H
