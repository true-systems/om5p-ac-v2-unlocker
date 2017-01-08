#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <environment.h>
#include <flash.h>
#include <malloc.h>
#include <net.h>
#include <md5.h>
#include <sha256.h>
#include <rsaverify.h>

/* placed at the end of the ART partition */
#define URSA_PUBKEY_BASE ((void *)(CFG_FLASH_BASE + 0xFF0000 + 0x8000))
#define URSA_PUBKEY_SIZE ((void *)(0x8000))

typedef struct _image_info_t {
	char *tagname;
	char *filename;
	char *filemd5sum;
	char *filesha256sum;
	char *md5sum;
	ulong flashaddr;
	int dataoffset;
	int datasize;
	int checksize;
	char *cmd_execute;
	char *cmd_success;
	char *cmd_fail;
	u_char checksum[33];
	u_char checksum_sha256[65];
	ulong load_addr;
	int load_size;
	struct _image_info_t *next;
} image_info_t;

extern env_t *env_ptr;
extern uchar *default_environment;
extern int default_environment_size;
extern void env_relocate_spec (void);

static int tftp_get (ulong loadaddr, char *filename)
{
	int rcode = 0;

	load_addr = loadaddr;
	copy_filename (BootFile, filename, sizeof(BootFile));
	if ((rcode = NetLoop (TFTPGET)) > 0) {
		flush_cache (load_addr, rcode);
	}

	return rcode;
}

static unsigned char *md5sum (char *buf, ulong addr, ulong size)
{
	static char hexchar[] = "0123456789abcdef";
	unsigned char temp[16] = {'\0'}, *s = buf;
	int i = -1;

	md5 ((unsigned char *)addr, size, temp);
	while (++i < sizeof(temp)) {
		*s++ = hexchar[(temp[i] >> 4) & 0xf];
		*s++ = hexchar[temp[i] & 0xf];
	}
	*s = '\0';

	return buf;
}

static unsigned char *sha256sum (char *buf, ulong addr, ulong size)
{
	static char hexchar[] = "0123456789abcdef";
	unsigned char temp[32] = {'\0'}, *s = buf;
	int i = -1;
	CSha256 m;

	Sha256_Init(&m);
	Sha256_Update(&m, (unsigned char *)addr, size);
	Sha256_Final(&m, temp);

	while (++i < sizeof(temp)) {
		*s++ = hexchar[(temp[i] >> 4) & 0xf];
		*s++ = hexchar[temp[i] & 0xf];
	}
	*s = '\0';

	return buf;
}

int do_datachk (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int rcode = 1;

	if (argc == 2) {
		char *arg[] = {argv[1], NULL};
		int i = 0;

		for (rcode = 0; 
			!rcode && arg[0] != NULL && ((arg[1] = strchr (arg[0], ',')) != NULL && (*arg[1] = '\0'), 1); arg[0] = arg[1] != NULL? arg[1] + 1: NULL) {
			if ((i = strlen (arg[0])) <= 16) {
				static char *str[] = {"start_addr", "size", "checksum"};
				char key[3][32], *addr = NULL, *size = NULL, *cksum = NULL;
				unsigned char calcksum[33];

				for (i = 0; i < 3 && (sprintf (key[i], "%s_%s", arg[0], str[i]) > 0 || !(rcode = 1)); i++);
				printf("Validating MD5Sum of '%s'... ", arg[0]);
				if (!rcode) {
					rcode = !((addr = getenv(key[0])) != NULL && (size = getenv(key[1])) != NULL && (cksum = getenv(key[2])) != NULL && 
						strcmp (cksum, md5sum (calcksum, simple_strtoul(addr, NULL, 16), simple_strtoul(size, NULL, 16))) == 0);
				}
				printf ("%s\n", !rcode? "Passed!": "Failed!");
				if (rcode && cksum != NULL) {
					printf ("    %s != %s\n", cksum, calcksum);
				}
			}
			else {
				printf ("Error: invalid tag length: %d!\n", i);
				rcode = 1;
			}
		}
		if (rcode) {
			printf ("\nData validation failed!\n\n");
		}
	}
	else {
		printf ("Usage:\n%s\n", cmdtp->usage);
	}
	
	return rcode;
}

U_BOOT_CMD(
 	datachk,	2,	1,	do_datachk,
 	"datachk - data validation using the MD5 hash\n",
	"[tagname[,tagname]...]\n"
);

int rsa_key_installed(void)
{
	bool ret;
	size_t r;

	/* validation public keys block */
	ret = validate_usign_block_keys(URSA_PUBKEY_BASE, URSA_PUBKEY_SIZE);
	if (!ret) {
		return 0;
	}

	r = get_usign_block_len(URSA_PUBKEY_BASE);
	if (r == 0)
		return 0;

	return 1;
}

static int validate_cfg_signature(const char *cfg_name, ulong dataaddr,
				  int datasize)
{
	char cfgname[256];
	ulong signatures_addr = dataaddr + datasize;
	int signatures_size;
	int retry = 3;
	struct crypthashes crypthashes;
	int ret;

	// TODO this should be: snprintf(cfgname, sizeof(cfgname), "%s.sig", cfg_name);
	sprintf(cfgname, "%s.sig", cfg_name);

	while (--retry > 0 && (signatures_size = tftp_get (signatures_addr, cfgname)) <= 0) {
		had_ctrlc ()? retry = 0: printf ("Retry%s\n", retry > 0? "...": " count exceeded!");
		WATCHDOG_RESET ();
	}

	if (retry == 0)
		return 0;

	/* validation signature block */
	ret = validate_usign_block_signatures(signatures_addr, signatures_size);
	if (!ret) {
		printf("Failed to validate sign block\n");
		return 0;
	}

	/* generate all known hashes */
	ret = generate_hashes(&crypthashes, dataaddr, datasize);
	if (ret < 0) {
		printf("Failed generate hashes\n");
		return 0;
	}

	/* WARNING it is assumed that pub is validated and memory mapped flash
	 * but it would also be possible to first load it from flash in a
	 * new buffer on stack/heap and then run this check on the new address
	 */
	return find_valid_signature(&crypthashes, URSA_PUBKEY_BASE,
				    signatures_addr);
}


int do_flashit (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int rcode = 1;
	int valid_cfg = 0;

	if (argc == 3) {
		env_t *eptr = NULL;
		char key[] = "netretry", *val = NULL, *s = NULL, *cmd_execute = NULL, *cmd_success = NULL, *cmd_fail = NULL;
		ulong dataaddr = 0;
		int dataoffset = 0, datasize = 0, retry = 3, f = 0;

		if ((s = getenv (key)) != NULL && (val = malloc (strlen (s) + 1)) != NULL) {
			strcpy (val, s);
		}
		setenv (key, "no");
		for (dataaddr = simple_strtoul(argv[1], NULL, 16); --retry > 0 && (datasize = tftp_get (dataaddr, argv[2])) <= 0;) {
			had_ctrlc ()? retry = 0: printf ("Retry%s\n", retry > 0? "...": " count exceeded!");
			WATCHDOG_RESET ();
		}

		/* check signature before parsing the cfg */
		if (retry > 0) {
			if (rsa_key_installed())
				valid_cfg = validate_cfg_signature(argv[2], dataaddr, datasize);
			else
				valid_cfg = 1;
		}

		if (valid_cfg) {
			char cmdbuf[CFG_CBSIZE];
			char *cptr[] = {(char *)dataaddr, (char *)dataaddr + datasize};
			char tmpchr[] = {'\0', '\0'};
			image_info_t *img_head = NULL, *img = NULL;

			for (s = NULL, ((char *)dataaddr)[datasize++] = '\n'; cptr[0] < cptr[1]; tmpchr[1] = *(cptr[0]++)) {
				switch (tmpchr[0] = *cptr[0]) {
					case '[':
					case ']':
					case '\r':
					case '\n':
						*cptr[0] = '\0';
						if(tmpchr[0] == '[') {
							if (tmpchr[1] == '\0') {
								f = 1;
								s = cptr[0] + 1;
							}
						}
						else {
							if (tmpchr[0] == ']') {
								image_info_t *tmpimg = (image_info_t *)malloc (sizeof(image_info_t));

								if (img_head == NULL) {
									img_head = img = tmpimg;
								}
								else {
									img = img->next = tmpimg;
								}
								img->tagname = s;
								img->filename = NULL;
								img->filemd5sum = NULL;
								img->filesha256sum = NULL;
								img->md5sum = NULL;
								img->flashaddr = CFG_FLASH_BASE + UBOOT_FLASH_SIZE;
								img->dataoffset = 0;
								img->datasize = 0;
								img->checksize = -1;
								img->cmd_execute = NULL;
								img->cmd_success = NULL;
								img->cmd_fail = NULL;
								img->checksum[32] = '\0';
								img->load_addr = 0;
								img->load_size = 0;
								img->next = NULL;
							}
							f = 0;
							s = NULL;
						}
						break;
					case '=':
						if (tmpchr[1] != '\0' && !f && s != NULL && img != NULL) {
							char *eol = NULL;

							*cptr[0]++ = '\0';
							if ((eol = strpbrk(cptr[0], "\r\n")) != NULL) {
								char *ptr = eol;

								while (*--ptr == ' ' || *ptr == '\t');
								ptr[1] = '\0';
							}
							if (strcmp (s, "filename") == 0) {
								img->filename = cptr[0];
							}
							else if (strcmp (s, "filemd5sum") == 0) {
								img->filemd5sum = cptr[0];
							}
							else if (strcmp (s, "filesha256sum") == 0) {
								img->filesha256sum = cptr[0];
							}
							else if (strcmp (s, "md5sum") == 0) {
								img->md5sum = cptr[0];
							}
							else if (strcmp (s, "flashaddr") == 0) {
								img->flashaddr = CFG_FLASH_BASE + simple_strtoul(cptr[0], NULL, 16);
							}
							else if (strcmp (s, "dataoffset") == 0) {
								img->dataoffset = simple_strtoul(cptr[0], NULL, 16);
							}
							else if (strcmp (s, "datasize") == 0) {
								img->datasize = simple_strtoul(cptr[0], NULL, 16);
							}
							else if (strcmp (s, "checksize") == 0) {
								img->checksize = simple_strtoul(cptr[0], NULL, 16);
							}
							else if (strcmp (s, "cmd_execute") == 0) {
								img->cmd_execute = cptr[0];
							}
							else if (strcmp (s, "cmd_success") == 0) {
								img->cmd_success = cptr[0];
							}
							else if (strcmp (s, "cmd_fail") == 0) {
								img->cmd_fail = cptr[0];
							}
							cptr[0] = eol;
							s = NULL;
						}
						break;
					default:
						if (tmpchr[1] == '\0' && !f) {
							s = cptr[0];
						}
						break;
				}//End of switch
			}//End of  for (s = NULL, ((char *)dataaddr)[datasize++] = '\n'; cptr[0] < cptr[1]; tmpchr[1] = *(cptr[0]++))
			for (img = img_head; retry > 0 && img != NULL; img = img->next) {
				WATCHDOG_RESET ();
				cmd_execute = img->cmd_execute;
				cmd_success = img->cmd_success;
				cmd_fail = img->cmd_fail;
				cmd_execute != NULL && run_command (cmd_execute, 0);
				if (img->flashaddr >= CFG_FLASH_BASE && img->flashaddr < CFG_FLASH_BASE + CFG_FLASH_SIZE) {
					if (img->filename != NULL) {
						for (retry = 3, dataaddr += datasize; retry-- > 0 && ((datasize = tftp_get (dataaddr, img->filename)) <= 0 ||
							//(img->filemd5sum != NULL && strcmp (img->filemd5sum, md5sum (img->checksum, dataaddr, datasize)) != 0));) {
							(img->filemd5sum != NULL && strcmp (img->filemd5sum, md5sum (img->checksum, dataaddr, datasize)) != 0) ||
							(img->filesha256sum != NULL && strcmp (img->filesha256sum, sha256sum (img->checksum_sha256, dataaddr, datasize)) != 0));) {
							had_ctrlc ()? retry = 0: printf ("Retry%s\n", retry > 0? "...": " count exceeded!");
						}
					}
					if (retry > 0) {
						img->load_addr = dataaddr;
						img->load_size = datasize;
						if (img->dataoffset < 0 || img->dataoffset > img->load_size) {
							img->dataoffset = 0;
						}
						if (img->datasize <= 0 || img->datasize > img->load_size) {
							img->datasize = img->load_size;
						}
						if (img->filename != NULL) {
							datasize = (dataoffset = img->dataoffset) + img->datasize;
						}
						if (img->dataoffset >= dataoffset && img->dataoffset + img->datasize <= img->load_size) {
							if (img->flashaddr + img->datasize <= CFG_FLASH_BASE + CFG_FLASH_SIZE) {
								if (img->checksize == 0 || img->checksize > img->datasize) {
									img->checksize = img->datasize;
								}

								if (img->md5sum) {
									strncpy(img->checksum, img->md5sum, sizeof(img->checksum));
									img->checksum[sizeof(img->checksum) - 1] = '\0';
								} else if (img->checksize >= 0 && (img->filename == NULL || img->filemd5sum == NULL ||
									img->dataoffset > 0 || img->checksize != img->load_size)) {
									md5sum (img->checksum, img->load_addr + img->dataoffset, img->checksize);
								}
							}
							else {
								printf ("Error: end address (0x%08x) not in flash!\nBad address format\n", img->flashaddr + img->datasize);
								retry = 0;
							}
						}
						else {
							printf ("Error: invalid data offset or size!\n");
							retry = 0;
						}
					}
				}
				else {
					printf ("Error: start address (0x%08x) not in flash!\nBad address format\n", img->flashaddr);
					retry = 0;
				}
			}//End of for (img = img_head; retry > 0 && img != NULL; img = img->next)
			for (img = img_head; retry > 0 && img != NULL; img = img->next) {
				WATCHDOG_RESET ();
				cmd_success = img->cmd_success;
				cmd_fail = img->cmd_fail;
				if (img->filename != NULL) {
					if (!flash_sect_protect (0, CFG_FLASH_BASE, CFG_FLASH_BASE + CFG_FLASH_SIZE) &&
						sprintf(cmdbuf, "erase 0x%x +0x%x", img->flashaddr, img->datasize) > 0 &&
						run_command (cmdbuf, 0) &&
						sprintf(cmdbuf, "cp.b 0x%x 0x%x 0x%x", img->load_addr + img->dataoffset, img->flashaddr, img->datasize) > 0 &&
						run_command (cmdbuf, 0)) {
						if (CFG_ENV_ADDR <= img->flashaddr + img->datasize && img->flashaddr < CFG_ENV_ADDR + CFG_ENV_SIZE) {
							if (crc32 (0, (eptr = (env_t *)CFG_ENV_ADDR)->data, ENV_SIZE) != eptr->crc) {
								puts ("*** Warning - bad CRC, using default environment\n\n");
								memset (env_ptr, 0, sizeof(env_t));
								memcpy (env_ptr->data, default_environment, default_environment_size);
								env_crc_update ();
								saveenv ();
								flash_sect_protect (0, CFG_FLASH_BASE, CFG_FLASH_BASE + CFG_FLASH_SIZE);
							}
							else {
								env_relocate_spec ();
							}
						}
					}
					else {
						retry = 0;
					}
				}
				if (retry > 0 && img->checksize >= 0) {
					sprintf(cptr[0] = cmdbuf,
						"%s_start_addr%c0x%08x%c"
						"%s_size%c0x%08x%c"
						"%s_checksum%c%s%c",
						img->tagname, '\0', img->flashaddr, '\0',
						img->tagname, '\0', img->checksize, '\0',
						img->tagname, '\0', img->checksum, '\0');
					while (*cptr[0] != '\0') {
						for (cptr[1] = cptr[0]; *cptr[1]++ != '\0';);
						setenv (cptr[0], cptr[1]);
						for (cptr[0] = cptr[1]; *cptr[0]++ != '\0';);
					}
				}
			}//End of for (img = img_head; retry > 0 && img != NULL; img = img->next)
			while ((img = img_head) != NULL) {
				img_head = img->next;
				free (img);
			}
			rcode = retry <= 0;
			setenv ("fileaddr", NULL);
			setenv ("filesize", NULL);
		}//End of if (valid_cfg)
		if (eptr == NULL) {
			setenv (key, val);
		}
		if (val != NULL) {
			free (val), val = NULL;
		}
		if (!rcode) {
			cmd_success != NULL && run_command (cmd_success, 0);
		}
		else {
			cmd_fail != NULL && run_command (cmd_fail, 0);
		}
	
	} //End if(argc == 3)
	else {
		printf ("Usage:\n%s\n", cmdtp->usage);
	}
	
	return rcode;
}

U_BOOT_CMD(
	flashit,	3,	1,	do_flashit,
	"flashit - batch write data into flash via network using TFTP protocol\n",
	"[loadAddress] [cfgfilename]\n"
);
