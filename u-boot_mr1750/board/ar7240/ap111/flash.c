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
    
    /* XXX this is hardcoded until we figure out how to read flash id */
    if(*(volatile int *)CFG_FLASH_MAGIC_F != CFG_FLASH_MAGIC) {
        flash_info->size = CFG_FLASH_SIZE; /* bytes */
        flash_info->sector_count = flash_info->size/CFG_FLASH_SECTOR_SIZE;
        for (i = 0; i < flash_info->sector_count; i++) {
            flash_info->start[i] = CFG_FLASH_BASE + (i * CFG_FLASH_SECTOR_SIZE);
            flash_info->protect[i] = 0;
        }
    }
    else {
        flash_info->size = CFG_FLASH_SIZE_F; /* bytes */
        flash_info->sector_count = flash_info->size/CFG_FLASH_SECTOR_SIZE_F;
        for (i = 0; i < flash_info->sector_count; i++) {
            flash_info->start[i] = CFG_FLASH_BASE + (i * CFG_FLASH_SECTOR_SIZE_F);
            flash_info->protect[i] = 0;
        }
    }

    printf ("sector count = %d\n", flash_info->sector_count);
    return (flash_info->size);

}
