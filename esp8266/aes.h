#ifdef __cplusplus
extern "C"{
#endif
#include <stdio.h>
#ifndef AES_H
#define AES_H

#define uchar unsigned char // 8-bit byte
#define uint unsigned long // 32-bit word
void KeyExpansion(uchar key[], uint w[], int keysize);
void aes_decrypt_1(uchar in[], uchar out[], uint key[], int keysize);
void aes_encrypt_1(uchar in[], uchar out[], uint key[], int keysize);

#endif


#ifdef __cplusplus
} // extern "C"
#endif
