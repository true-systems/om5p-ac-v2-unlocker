#
# Copyright (C) 2017 Piotr Dymacz <piotr@dymacz.pl>
#

-include local-settings.mk
include utils.mk

ifndef CROSS_COMPILE
$(error Please define CROSS_COMPILE environment variable)
endif

export CROSS_COMPILE
export BUILD_TOPDIR = $(PWD)
export STAGING_DIR  = $(BUILD_TOPDIR)/tmp
export OUTPUT_DIR   = $(BUILD_TOPDIR)/output
export SOURCE_DIR   = $(BUILD_TOPDIR)/u-boot_mr1750

OM5PACV2_RAM_UBOOT_PARAMS = BOARD_NAME=OM5P-ACv2 CFG_BOARD_TYPE=ap135 \
                            CONFIG_SKIP_LOWLEVEL_INIT=1 ETH_CONFIG2=_ar8033 \
                            CONFIG_OM5PACV2_UNLOCKER=1 FLASH_SIZE=16
# =======
# TARGETS
# =======

om5p-ac-v2-unlocker: UBOOT_TARGET_CONFIG=board955x_config
om5p-ac-v2-unlocker: UBOOT_TARGET_PARAMS=$(OM5PACV2_RAM_UBOOT_PARAMS)
om5p-ac-v2-unlocker: $(OUTPUT_DIR)/fwupgrade.cfg $(OUTPUT_DIR)/fwupgrade.cfg.sig

$(OUTPUT_DIR)/fwupgrade.cfg.sig:
	@$(call gen_zero_file,fwupgrade.cfg.sig,132413292)
	@$(call MESSAGE,Preparing exploit instructions inside '$(notdir $@)'...)
	@$(call swap_byte,$(OUTPUT_DIR)/fwupgrade.cfg.sig,132413288,0x80)
	@$(call swap_byte,$(OUTPUT_DIR)/fwupgrade.cfg.sig,132413289,0x13)
	@$(call swap_byte,$(OUTPUT_DIR)/fwupgrade.cfg.sig,280,0x80)
	@$(call swap_byte,$(OUTPUT_DIR)/fwupgrade.cfg.sig,281,0x13)
	@$(call swap_byte,$(OUTPUT_DIR)/fwupgrade.cfg.sig,528,0x80)
	@$(call swap_byte,$(OUTPUT_DIR)/fwupgrade.cfg.sig,529,0xf)
	@$(call swap_byte,$(OUTPUT_DIR)/fwupgrade.cfg.sig,530,0x90)
	@$(call swap_byte,$(OUTPUT_DIR)/fwupgrade.cfg.sig,531,0x80)

$(SOURCE_DIR)/include/config.h:
	@$(call MESSAGE,Configuring U-Boot for '$(UBOOT_TARGET_CONFIG)'...)
	@$(call build_uboot,$(UBOOT_TARGET_CONFIG))

$(SOURCE_DIR)/u-boot.bin: $(SOURCE_DIR)/include/config.h
	@$(call MESSAGE,Building RAM loadable U-Boot image...)
	@$(call build_uboot,all)

$(OUTPUT_DIR)/fwupgrade.cfg: $(SOURCE_DIR)/u-boot.bin
	@$(call gen_zero_file,fwupgrade.cfg,196608)
	@$(call combine_files,$(SOURCE_DIR)/u-boot.bin,$(OUTPUT_DIR)/fwupgrade.cfg)

clean:
	@$(call MESSAGE,Cleaning...)
	$(SILENT) cd $(SOURCE_DIR) && $(SUBMAKE) distclean
	$(SILENT) rm -f $(OUTPUT_DIR)/fwupgrade.cfg*

include circle.mk
