# ROM version
ifdef BOOT_FROM_NAND #{
ifeq ($(COMPRESSED_UBOOT),1) #{
TEXT_BASE = 0x80010000
BOOTSTRAP_TEXT_BASE = 0xa0100000
else #}{
TEXT_BASE = 0xa0100000
endif #}
else #}{
ifeq ($(COMPRESSED_UBOOT),1) #{
TEXT_BASE = 0x80010000
BOOTSTRAP_TEXT_BASE = 0x9f000000
else #}{
TEXT_BASE = 0x9f000000
endif #}
endif #}

ifdef CONFIG_SKIP_LOWLEVEL_INIT
	TEXT_BASE = 0x80100000
else
	TEXT_BASE = 0x9F000000
endif

export BOARD_EXTRA_OBJS_tmp =

BOARD_EXTRA_OBJS_tmp += $(TOPDIR)/board/$(BOARDDIR)/extra.o
ifeq ($(BOOT_FROM_NAND),1)
BOARD_EXTRA_OBJS_tmp += $(TOPDIR)/board/$(VENDOR)/common/ath_nand.o $(TOPDIR)/drivers/nand/nand_ids.o
endif

ifeq ($(COMPRESSED_UBOOT),1) #{
BOARD_EXTRA_OBJS = $(BOARD_EXTRA_OBJS_tmp)
endif
