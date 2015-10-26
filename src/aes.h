// slightly modified
/*********************************************************************
* Filename:   aes.h
* Author:     Brad Conte (brad AT bradconte.com)
* Copyright:
* Disclaimer: This code is presented "as is" without any guarantees.
* Details:    Defines the API for the corresponding AES implementation.
*********************************************************************/

#ifndef AES_H
#define AES_H

#include <stdint.h>

/****************************** MACROS ******************************/
#define AES_BLOCK_SIZE 16               // AES operates on 16 bytes at a time


/*********************** FUNCTION DECLARATIONS **********************/
///////////////////
// AES
///////////////////
// Key setup must be done before any AES en/de-cryption functions can be used.
void aes_key_setup(const uint8_t key[],          // The key, must be 128, 192, or 256 bits
                   uint32_t w[],                  // Output key schedule to be used later
                   int keysize);              // Bit length of the key, 128, 192, or 256

void aes_encrypt(const uint8_t in[],             // 16 bytes of plaintext
                 uint8_t out[],                  // 16 bytes of ciphertext
                 const uint32_t key[],            // From the key setup
                 int keysize);                // Bit length of the key, 128, 192, or 256

void aes_decrypt(const uint8_t in[],             // 16 bytes of ciphertext
                 uint8_t out[],                  // 16 bytes of plaintext
                 const uint32_t key[],            // From the key setup
                 int keysize);                // Bit length of the key, 128, 192, or 256

#endif   // AES_H
