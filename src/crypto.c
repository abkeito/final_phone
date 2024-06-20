// crypto.c
#include "crypto.h"
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>

static const unsigned char *key = (unsigned char *)"01234567890123456789012345678901"; // 32 bytes key for AES-256
static const unsigned char *iv = (unsigned char *)"0123456789012345"; // 16 bytes IV

void handleErrors(void)
{
  ERR_print_errors_fp(stderr);
  abort();
}

int myencrypt(unsigned char *plaintext, int plaintext_len, unsigned char *ciphertext)
{
  EVP_CIPHER_CTX *ctx;
  int len;
  int ciphertext_len;

  // Create and initialize the context
  if (!(ctx = EVP_CIPHER_CTX_new()))
    handleErrors();

  // Initialize the encryption operation.
  if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
    handleErrors();

  // Provide the message to be encrypted, and obtain the encrypted output.
  if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
    handleErrors();
  ciphertext_len = len;

  // Finalize the encryption.
  if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
    handleErrors();
  ciphertext_len += len;

  // Clean up
  EVP_CIPHER_CTX_free(ctx);

  return ciphertext_len;
}

int mydecrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *plaintext)
{
  EVP_CIPHER_CTX *ctx;
  int len;
  int plaintext_len;

  // Create and initialize the context
  if (!(ctx = EVP_CIPHER_CTX_new()))
    handleErrors();

  // Initialize the decryption operation.
  if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
    handleErrors();

  // Provide the message to be decrypted, and obtain the plaintext output.
  if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
    handleErrors();
  plaintext_len = len;

  // Finalize the decryption.
  if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
    handleErrors();
  plaintext_len += len;

  // Clean up
  EVP_CIPHER_CTX_free(ctx);

  return plaintext_len;
}
