/* 2010-06-11 : Igor Pavlov : Public domain */

#ifndef _SHA256_H_
#define _SHA256_H_

#include <stddef.h>
#if defined(__KERNEL__) || defined(__U_BOOT__)
#include <linux/types.h>
#else
#include <stdint.h>
#endif

typedef struct {
  uint32_t state[8];
  uint64_t count;
  uint8_t buffer[64];
} CSha256;

void Sha256_Init(CSha256 *p);
void Sha256_Update(CSha256 *p, const uint8_t *data, size_t size);
void Sha256_Final(CSha256 *p, uint8_t *digest);

#endif
