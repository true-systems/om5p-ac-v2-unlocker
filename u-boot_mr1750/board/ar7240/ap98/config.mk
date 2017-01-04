# ROM version

ifeq ($(COMPRESSED_UBOOT),1)
# change the text base to ram address
TEXT_BASE = 0x80310000
# set to BOOTSTRAP text base to cached base address
BOOTSTRAP_TEXT_BASE = 0x9f000000
else
TEXT_BASE = 0x9f000000
endif

# SDRAM version
#TEXT_BASE = 0x80000000
