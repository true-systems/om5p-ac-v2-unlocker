/*
 * Copyright (c) 2013, Google Inc.
 * Copyright (c) 2015, Open Mesh Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _URSA_H
#define _URSA_H

#if defined(__KERNEL__) || defined(__U_BOOT__)
#include <linux/types.h>
#include <linux/string.h>
#else
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#endif
#include <stddef.h>

#define RSA2048_BYTES	(2048 / 8)
#define RSA4096_BYTES	(4096 / 8)

/* This is the minimum/maximum key size we support, in bits */
#define RSA_MIN_KEY_BITS	2048
#define RSA_MAX_KEY_BITS	4096

/* This is the maximum signature length that we support, in bits */
#define RSA_MAX_SIG_BITS	4096
#define SHA256_SUM_LEN		32
#define SHA1_SUM_LEN		20

extern const uint8_t padding_sha256_rsa2048[RSA2048_BYTES - SHA256_SUM_LEN];
extern const uint8_t padding_sha1_rsa2048[RSA2048_BYTES - SHA1_SUM_LEN];
extern const uint8_t padding_sha256_rsa4096[RSA4096_BYTES - SHA256_SUM_LEN];

struct rsa_public_key {
	unsigned int len;	/* len of modulus[] in number of uint32_t */
	uint32_t n0inv;		/* -1 / modulus[0] mod 2^32 */
	uint32_t *modulus;	/* modulus as little endian array */
	uint32_t *rr;		/* R^2 as little endian array */
	uint64_t exponent;	/* public exponent */
};

/**
 * struct key_prop - holder for a public key properties
 *
 * The struct has pointers to modulus (Typically called N),
 * The inverse, R^2, exponent. These can be typecasted and
 * used as byte arrays or converted to the required format
 * as per requirement of RSA implementation.
 */
struct key_prop {
	const void *rr;		/* R^2 can be treated as byte array */
	const void *modulus;	/* modulus as byte array */
	const void *public_exponent; /* public exponent as byte array */
	uint32_t n0inv;		/* -1 / modulus[0] mod 2^32 */
	int num_bits;		/* Key length in bits */
};

struct crypthashes {
	uint8_t sha256[SHA256_SUM_LEN];
};

#define UINT64_MULT32(v, multby)  (((uint64_t)(v)) * ((uint32_t)(multby)))

#ifndef debug
//#define debug(...) fprintf(stderr, __VA_ARGS__)
#define debug(...)
#endif

/* slow but safe version for older uboot */
static inline uint16_t get_unaligned_be16(const void *ptr)
{
	uint16_t res = 0;
	size_t i;
	const uint8_t *a = ptr;

	for (i = 0; i < sizeof(res); i++) {
		res <<= 8;
		res |= a[i];
	}

	return res;
}

/* slow but safe version for older uboot */
static inline uint32_t get_unaligned_be32(const void *ptr)
{
	uint32_t res = 0;
	size_t i;
	const uint8_t *a = ptr;

	for (i = 0; i < sizeof(res); i++) {
		res <<= 8;
		res |= a[i];
	}

	return res;
}

/* slow but safe version for older uboot */
static inline uint64_t get_unaligned_be64(const void *ptr)
{
	uint64_t res = 0;
	size_t i;
	const uint8_t *a = ptr;

	for (i = 0; i < sizeof(res); i++) {
		res <<= 8;
		res |= a[i];
	}

	return res;
}

/* slow but safe version for older uboot */
static inline void put_unaligned_be32(uint32_t a, void *ptr)
{
	size_t i;
	uint8_t *b = ptr;

	for (i = 0; i < sizeof(a); i++) {
		b[sizeof(a) - i - 1]  = a & 0xff;
		a >>= 8;
	}
}

int rsa_mod_exp_sw(const uint8_t *sig, uint32_t sig_len,
		struct key_prop *prop, uint8_t *out);

#endif /* _URSA_H */
