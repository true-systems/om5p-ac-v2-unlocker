/*
 * Copyright (c) 2015, Open Mesh Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <stdbool.h>

#if defined(__KERNEL__) || defined(__U_BOOT__)
#include <linux/types.h>
#else
#include <stdio.h>
#include <stdlib.h>
#endif
#include "uboot_sign.h"
#include "ursa.h"
#include "sha256.h"

long get_usign_block_len(const void *b)
{
	uint32_t length;
	const struct usign_tlv *tlv = b;

	length = get_unaligned_be32(&tlv->length);

	return length;
}

static bool validate_usign_block_len(const void *b, long len)
{
	uint32_t length;
	const struct usign_tlv *tlv = b;

	/* has tlv */
	if (len < (long)sizeof(*tlv))
		return false;

	length = get_usign_block_len(b);

	if ((len - sizeof(*tlv)) < length)
		return false;

	return true;
}

bool validate_usign_block_keys(const void *b, long len)
{
	uint16_t type;
	uint16_t version;
	const struct usign_block *block = b;

	/* has tlv */
	if (!validate_usign_block_len(b, len))
		return false;

	type = get_unaligned_be16(&block->header.type);
	version = get_unaligned_be16(&block->header.version);

	if (type != USIGN_BLOCK_KEYS)
		return false;

	switch (version) {
	case USIGN_BLOCK_KEYS_V1:
		break;
	default:
		return false;
	}

	return true;
}

static bool validate_usign_block_key(const void *b, long len)
{
	uint16_t type;
	uint16_t version;
	const struct usign_block_key *block = b;

	/* has tlv */
	if (!validate_usign_block_len(b, len))
		return false;

	type = get_unaligned_be16(&block->header.type);
	version = get_unaligned_be16(&block->header.version);

	switch (type) {
	case USIGN_BLOCK_KEY_RSA:
		switch (version) {
		case USIGN_BLOCK_KEY_RSA_V1:
			break;
		default:
			return false;
		}
		break;
	default:
		return false;
	}

	return true;
}

bool validate_usign_block_signatures(const void *b, long len)
{
	uint16_t type;
	uint16_t version;
	const struct usign_block *block = b;

	/* has tlv */
	if (!validate_usign_block_len(b, len))
		return false;

	type = get_unaligned_be16(&block->header.type);
	version = get_unaligned_be16(&block->header.version);

	if (type != USIGN_BLOCK_SIGNATURES)
		return false;

	switch (version) {
	case USIGN_BLOCK_SIGNATURES_V1:
		break;
	default:
		return false;
	}

	return true;
}

static bool validate_usign_block_signature(const const void *b, long len)
{
	uint16_t type;
	uint16_t version;
	const struct usign_block_signature *block = b;

	/* has tlv */
	if (!validate_usign_block_len(b, len))
		return false;

	type = get_unaligned_be16(&block->header.type);
	version = get_unaligned_be16(&block->header.version);

	switch (type) {
	case USIGN_BLOCK_SIGNATURE_RSASHA256:
		switch (version) {
		case USIGN_BLOCK_SIGNATURE_RSASHA256_V1:
			break;
		default:
			return false;
		}
		break;
	default:
		return false;
	}

	return true;
}

static int generate_hashes_sha256(uint8_t sha256[SHA256_SUM_LEN],
				  const void *blob, size_t len)
{
	CSha256 m;

	Sha256_Init(&m);
	Sha256_Update(&m, blob, len);
	Sha256_Final(&m, sha256);

	return 0;
}

int generate_hashes(struct crypthashes *hashes, const void *blob,
		    size_t len)
{
	int ret;

	ret = generate_hashes_sha256(hashes->sha256, blob, len);
	if (ret < 0)
		return ret;

	return 0;
}

static bool check_key_signature_rsa_v1_sha256_v1(const struct crypthashes *hashes,
						 const struct usign_block_key *block_key,
						 const struct usign_block_signature *block_signature)
{
	long rsasha256len;
	long expected_siglen;
	long rsalen;
	const struct usign_block_key_rsa *block_key_rsa;
	uint32_t rsa_bits;
	uint32_t rsa_n0_inv;
	long mod_r2_len;
	long mod_len;
	struct key_prop prop;
	int ret;
	const uint8_t *padding;
	size_t pad_len;

	/* don't accept rsa key without the normal, static header */
	rsalen = get_usign_block_len(block_key);
	if (rsalen < (long)sizeof(*block_key_rsa))
		return false;

	block_key_rsa = (const struct usign_block_key_rsa *)block_key->data;
	rsa_bits = get_unaligned_be32(&block_key_rsa->bits);
	rsa_n0_inv = get_unaligned_be32(&block_key_rsa->n0_inv);

	switch (rsa_bits) {
	case 2048:
		mod_len = 256;
		expected_siglen = 256;
		padding = padding_sha256_rsa2048;
		break;
	case 4096:
		mod_len = 512;
		expected_siglen = 512;
		padding = padding_sha256_rsa4096;
		break;
	default:
		return false;
	};

	mod_r2_len = mod_len * 2;
	pad_len = expected_siglen - SHA256_SUM_LEN;

	/* don't accept rsa key without the normal, static + flexible data */
	if (rsalen != (long)sizeof(*block_key_rsa) + mod_r2_len)
		return false;

	/* only test signature with right size */
	rsasha256len = get_usign_block_len(block_signature);
	if (rsasha256len != expected_siglen)
		return false;

	uint8_t buf[expected_siglen];

	prop.modulus = block_key->data + sizeof(*block_key_rsa);
	prop.rr = block_key->data + sizeof(*block_key_rsa) + mod_len;
	prop.public_exponent = &block_key_rsa->exponent;
	prop.n0inv = rsa_n0_inv;
	prop.num_bits = rsa_bits;

	ret = rsa_mod_exp_sw(block_signature->data, rsasha256len, &prop, buf);
	if (ret)
		return false;

	/* check if padding was decrypted correctly */
	if (memcmp(buf, padding, pad_len) != 0)
		return false;

	/* check if checksum was decrypted correctly */
	if (memcmp(buf + pad_len, hashes->sha256, SHA256_SUM_LEN) != 0)
		return false;

	return true;
}

static bool check_key_signature_rsa_sha256(const struct crypthashes *hashes,
					   const struct usign_block_key *block_key,
					   const struct usign_block_signature *block_signature)
{
	uint16_t key_version;
	uint16_t signature_version;

	key_version = get_unaligned_be16(&block_key->header.version);
	signature_version = get_unaligned_be16(&block_signature->header.version);

	switch (key_version) {
	case USIGN_BLOCK_KEY_RSA_V1:
		switch (signature_version) {
		case USIGN_BLOCK_SIGNATURE_RSASHA256_V1:
			return check_key_signature_rsa_v1_sha256_v1(hashes,
								    block_key,
								    block_signature);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	};

	return true;
}

static bool check_key_signature(const struct crypthashes *hashes,
				const struct usign_block_key *block_key,
				const struct usign_block_signature *block_signature)
{
	uint16_t key_type;
	uint16_t signature_type;

	key_type = get_unaligned_be16(&block_key->header.type);
	signature_type = get_unaligned_be16(&block_signature->header.type);

	switch (key_type) {
	case USIGN_BLOCK_KEY_RSA:
		switch (signature_type) {
		case USIGN_BLOCK_SIGNATURE_RSASHA256:
			return check_key_signature_rsa_sha256(hashes,
							      block_key,
							      block_signature);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	};

	return false;
}

bool find_valid_signature(const struct crypthashes *hashes,
				 const void *pub, const void *sign)
{
	size_t pub_pos;
	size_t pub_len;
	size_t sign_pos;
	size_t sign_len;
	const struct usign_block *pub_block = pub;
	const struct usign_block *sign_block = sign;
	const uint8_t *cur_key;
	const uint8_t *cur_signature;
	long cur_key_len;
	long cur_key_block_len;
	long cur_signature_len;
	long cur_signature_block_len;
	bool valid;
	uint16_t pub_version;
	uint16_t sign_version;
	const struct usign_block_key *block_key;
	const struct usign_block_signature *block_signature;

	pub_version = get_unaligned_be16(&pub_block->header.version);
	sign_version = get_unaligned_be16(&sign_block->header.version);

	/* only v1 signatures block and v1 keys block is supported in this
	 * algorithm
	 */
	if (pub_version != USIGN_BLOCK_KEYS_V1 ||
	    sign_version != USIGN_BLOCK_SIGNATURES_V1)
		return false;

	pub_len = get_usign_block_len(pub);
	sign_len = get_usign_block_len(sign);

	/* go through all keys and signatures to find a valid one */
	pub_pos = 0;
	cur_key = pub_block->data;
	while (pub_pos < pub_len) {
		if (!validate_usign_block_len(cur_key, pub_len - pub_pos))
			return false;

		cur_key_len = get_usign_block_len(cur_key);
		cur_key_block_len = cur_key_len + sizeof(struct usign_tlv);

		/* skip keys we don't understand */
		if (!validate_usign_block_key(cur_key, cur_key_block_len)) {
			pub_pos += cur_key_block_len;
			cur_key += cur_key_block_len;
			continue;
		}

		block_key = (const struct usign_block_key *)cur_key;

		sign_pos = 0;
		cur_signature = sign_block->data;
		while (sign_pos < sign_len) {
			if (!validate_usign_block_len(cur_signature,
						      sign_len - sign_pos))
				return false;

			cur_signature_len = get_usign_block_len(cur_signature);
			cur_signature_block_len = cur_signature_len + sizeof(struct usign_tlv);

			/* skip signatures we don't understand */
			if (!validate_usign_block_signature(cur_signature,
							    cur_signature_block_len)) {
				sign_pos += cur_signature_block_len;
				cur_signature += cur_signature_block_len;
				continue;
			}

			block_signature = (const struct usign_block_signature *)cur_signature;
			valid = check_key_signature(hashes, block_key,
						    block_signature);
			if (valid)
				return true;

			/* no success; switch to next signature */
			sign_pos += cur_signature_block_len;
			cur_signature += cur_signature_block_len;
		}

		/* no success; switch to next key */
		pub_pos += cur_key_block_len;
		cur_key += cur_key_block_len;
	}

	return false;
}

