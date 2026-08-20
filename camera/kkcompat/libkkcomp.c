/* Compatibility hooks required by Smartisan's KitKat camera stack. */

#include <limits.h>
#include <stddef.h>

#include <openssl/cipher.h>
#include <openssl/evp.h>
#include <openssl/mem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>

/*
 * Bionic used to export the ARM EABI division-by-zero handler. Modern bionic
 * no longer does, but the legacy camera libraries still reference it.
 */
int __aeabi_idiv0(int return_value)
{
    return return_value;
}

/*
 * BoringSSL removed OpenSSL's envelope helpers. The stock JPEG codec imports
 * them even when encrypted MOBICAT metadata is not configured, so provide the
 * old API in terms of BoringSSL's supported EVP primitives.
 */
int EVP_SealInit(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *cipher,
                 unsigned char **encrypted_keys, int *encrypted_key_lengths,
                 unsigned char *iv, EVP_PKEY **public_keys,
                 int public_key_count)
{
    unsigned char key[EVP_MAX_KEY_LENGTH];
    unsigned int key_length;
    unsigned int iv_length;
    int result = 0;
    int i;

    if (ctx == NULL || cipher == NULL || encrypted_keys == NULL ||
            encrypted_key_lengths == NULL || public_keys == NULL ||
            public_key_count <= 0) {
        return 0;
    }

    key_length = EVP_CIPHER_key_length(cipher);
    iv_length = EVP_CIPHER_iv_length(cipher);
    if (key_length == 0 || key_length > sizeof(key) ||
            RAND_bytes(key, key_length) != 1 ||
            (iv_length != 0 &&
             (iv == NULL || RAND_bytes(iv, iv_length) != 1))) {
        goto out;
    }

    for (i = 0; i < public_key_count; ++i) {
        EVP_PKEY_CTX *pkey_ctx;
        size_t output_length;

        encrypted_key_lengths[i] = 0;
        if (encrypted_keys[i] == NULL || public_keys[i] == NULL) {
            goto out;
        }

        pkey_ctx = EVP_PKEY_CTX_new(public_keys[i], NULL);
        if (pkey_ctx == NULL) {
            goto out;
        }

        output_length = (size_t)EVP_PKEY_size(public_keys[i]);
        if (EVP_PKEY_encrypt_init(pkey_ctx) != 1 ||
                EVP_PKEY_CTX_set_rsa_padding(pkey_ctx,
                                             RSA_PKCS1_PADDING) != 1 ||
                EVP_PKEY_encrypt(pkey_ctx, encrypted_keys[i], &output_length,
                                 key, key_length) != 1 ||
                output_length > INT_MAX) {
            EVP_PKEY_CTX_free(pkey_ctx);
            goto out;
        }

        EVP_PKEY_CTX_free(pkey_ctx);
        encrypted_key_lengths[i] = (int)output_length;
    }

    if (EVP_EncryptInit_ex(ctx, cipher, NULL, key, iv) == 1) {
        result = public_key_count;
    }

out:
    OPENSSL_cleanse(key, sizeof(key));
    return result;
}

int EVP_SealFinal(EVP_CIPHER_CTX *ctx, unsigned char *output,
                  int *output_length)
{
    return EVP_EncryptFinal_ex(ctx, output, output_length);
}

int EVP_OpenInit(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *cipher,
                 const unsigned char *encrypted_key,
                 int encrypted_key_length, const unsigned char *iv,
                 EVP_PKEY *private_key)
{
    unsigned char key[EVP_MAX_KEY_LENGTH];
    EVP_PKEY_CTX *pkey_ctx = NULL;
    size_t key_length = sizeof(key);
    unsigned int expected_key_length;
    int result = 0;

    if (ctx == NULL || cipher == NULL || encrypted_key == NULL ||
            encrypted_key_length <= 0 || private_key == NULL) {
        return 0;
    }

    expected_key_length = EVP_CIPHER_key_length(cipher);
    pkey_ctx = EVP_PKEY_CTX_new(private_key, NULL);
    if (expected_key_length == 0 || expected_key_length > sizeof(key) ||
            pkey_ctx == NULL || EVP_PKEY_decrypt_init(pkey_ctx) != 1 ||
            EVP_PKEY_CTX_set_rsa_padding(pkey_ctx, RSA_PKCS1_PADDING) != 1 ||
            EVP_PKEY_decrypt(pkey_ctx, key, &key_length, encrypted_key,
                             (size_t)encrypted_key_length) != 1 ||
            key_length != expected_key_length) {
        goto out;
    }

    result = EVP_DecryptInit_ex(ctx, cipher, NULL, key, iv);

out:
    EVP_PKEY_CTX_free(pkey_ctx);
    OPENSSL_cleanse(key, sizeof(key));
    return result;
}

int EVP_OpenFinal(EVP_CIPHER_CTX *ctx, unsigned char *output,
                  int *output_length)
{
    return EVP_DecryptFinal_ex(ctx, output, output_length);
}
