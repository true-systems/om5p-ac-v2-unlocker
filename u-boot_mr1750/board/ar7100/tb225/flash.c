#include <common.h>
#include <config.h>
#include <asm/types.h>
#include <flash.h>

/*
 * sets up flash_info and returns size of FLASH (bytes)
 */
unsigned long 
flash_get_geom (flash_info_t *flash_info)
{
    int i;

    flash_info->flash_id  = FLASH_M25P64;
    flash_info->size      = (CFG_MAX_FLASH_BANKS * CFG_MAX_FLASH_SECT *
                            CFG_FLASH_SECTOR_SIZE);
    flash_info->sector_count = CFG_MAX_FLASH_SECT;

    for (i = 0; i < flash_info->sector_count; i++) {
        flash_info->start[i] = CFG_FLASH_BASE + (i * CFG_FLASH_SECTOR_SIZE);
        flash_info->protect[i] = 0;
    }

	printf ("flash size %dMB\n", flash_info->size/(1024*1024));
	return (flash_info->size);
}
