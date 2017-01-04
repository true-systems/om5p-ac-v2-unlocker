#ifndef _UBOOT_SIGN_H
#define _UBOOT_SIGN_H

#if defined(__KERNEL__) || defined(__U_BOOT__)
#include <linux/types.h>
#else
#include <stdint.h>
#endif

#define __packed __attribute__ ((packed))

struct usign_tlv {
	uint16_t type;
	uint16_t version;
	uint32_t length;
} __packed;

struct usign_block {
	struct usign_tlv header;
	__extension__ uint8_t data[0];
} __packed;

enum usign_block_type {
	USIGN_BLOCK_KEYS = 0,
	USIGN_BLOCK_SIGNATURES = 1,
};


#define USIGN_BLOCK_KEYS_V1 1

struct usign_block_key {
	struct usign_tlv header;
	__extension__ uint8_t data[0];
} __packed;

enum usign_block_key_type {
	USIGN_BLOCK_KEY_RSA = 0,
};


#define USIGN_BLOCK_KEY_RSA_V1 1

struct usign_block_key_rsa {
	uint32_t bits;
	uint32_t n0_inv;
	uint64_t exponent;
	__extension__ uint8_t data[0]; /* modulus, r-squared  with bits each */
} __packed;


#define USIGN_BLOCK_SIGNATURES_V1 1

struct usign_block_signature {
	struct usign_tlv header;
	__extension__ uint8_t data[0];
} __packed;

enum usign_block_signature_type {
	USIGN_BLOCK_SIGNATURE_RSASHA256 = 0,
};


#define USIGN_BLOCK_SIGNATURE_RSASHA256_V1 1

/* just raw bytes for usign_block_signature_rsasha256 */

#endif /* _UBOOT_SIGN_H */

