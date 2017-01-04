/*
 * Copyright (c) 2015, Open Mesh Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _RSAVERIFY_H
#define _RSAVERIFY_H

#include <stdbool.h>

#include "uboot_sign.h"
#include "ursa.h"

long get_usign_block_len(const void *b);
bool validate_usign_block_keys(const void *b, long len);
bool validate_usign_block_signatures(const void *b, long len);
int generate_hashes(struct crypthashes *hashes, const void *blob,
		    size_t len);
bool find_valid_signature(const struct crypthashes *hashes,
				 const void *pub, const void *sign);

/* see cmd_custom.c */
int rsa_key_installed(void);

#endif /* _RSAVERIFY_H */
