#
# Copyright (C) 2017 Piotr Dymacz <piotr@dymacz.pl>
#

SHELL = bash

# Define _absolute_ path to your toolchain directory, for example:
# export TOOLCHAIN_DIR:=/home/user/toolchain-mips_24kc_gcc-5.4.0_musl-1.1.15
# export PATH:=$(TOOLCHAIN_DIR)/bin:$(PATH)

ifndef CROSS_COMPILE
  CROSS_COMPILE = mips-openwrt-linux-musl-
endif
export CROSS_COMPILE

export BUILD_TOPDIR = $(PWD)
export STAGING_DIR  = $(BUILD_TOPDIR)/tmp
export OUTPUT_DIR   = $(BUILD_TOPDIR)/output
export SOURCE_DIR   = $(BUILD_TOPDIR)/u-boot_mr1750
export SUB_MAKE_CMD = $(MAKE) --silent --no-print-directory V=1 \
                      ARCH=mips STAGING_DIR=$(STAGING_DIR) \
                      COMPRESSED_UBOOT=0 CROSS_COMPILE=$(CROSS_COMPILE)

OM5PACV2_RAM_UBOOT_PARAMS = BOARD_NAME=OM5P-ACv2 CFG_BOARD_TYPE=ap135 \
                            CONFIG_SKIP_LOWLEVEL_INIT=1 ETH_CONFIG2=_ar8033 \
                            CONFIG_OM5PACV2_UNLOCKER=1 FLASH_SIZE=16

# ================
# HELPER FUNCTIONS
# ================

define echo_msg
  echo -e "\e[92m$(1)\e[0m"
endef

# $(1): filename
# $(2): size
define gen_zero_file
  $(call echo_msg,Generating empty file '$(1)' (size '$(2)')...)
  if [ -e $(OUTPUT_DIR)/$(1) ]; then \
    rm -f $(OUTPUT_DIR)/$(1); \
  fi; \
  dd if=/dev/zero of=$(OUTPUT_DIR)/$(1) bs=$(2) count=1 >/dev/null 2>&1
endef

# $(1): target
define build_uboot
  cd $(SOURCE_DIR) && \
     $(SUB_MAKE_CMD) $(strip $(1)) $(UBOOT_TARGET_PARAMS) >/dev/null 2>&1
endef

# $(1): input file
# $(2): output file
define combine_files
  @$(call echo_msg,Combining '$(notdir $1)' and '$(notdir $2)' files...)
  dd if=$(1) of=$(2) bs=$(shell wc -c < $(1)) \
     count=1 conv=notrunc >/dev/null 2>&1
endef

# $(1): input file
# $(2): offset
# $(3): value
define swap_byte
  printf "$(shell printf '\\x%02X' $(3))" |\
  dd of=$(1) bs=1 count=1 seek=$(2) conv=notrunc >/dev/null 2>&1
endef

# =======
# TARGETS
# =======

$(OUTPUT_DIR)/fwupgrade.cfg.sig:
	@$(call gen_zero_file,fwupgrade.cfg.sig,132413292)
	@$(call echo_msg,Preparing exploit instructions inside '$(notdir $@)'...)
	@$(call swap_byte,$(OUTPUT_DIR)/fwupgrade.cfg.sig,132413288,0x80)
	@$(call swap_byte,$(OUTPUT_DIR)/fwupgrade.cfg.sig,132413289,0x13)
	@$(call swap_byte,$(OUTPUT_DIR)/fwupgrade.cfg.sig,280,0x80)
	@$(call swap_byte,$(OUTPUT_DIR)/fwupgrade.cfg.sig,281,0x13)
	@$(call swap_byte,$(OUTPUT_DIR)/fwupgrade.cfg.sig,528,0x80)
	@$(call swap_byte,$(OUTPUT_DIR)/fwupgrade.cfg.sig,529,0xf)
	@$(call swap_byte,$(OUTPUT_DIR)/fwupgrade.cfg.sig,530,0x90)
	@$(call swap_byte,$(OUTPUT_DIR)/fwupgrade.cfg.sig,531,0x80)

$(SOURCE_DIR)/include/config.h:
	@$(call echo_msg,Configuring U-Boot for '$(UBOOT_TARGET_CONFIG)'...)
	@$(call build_uboot,$(UBOOT_TARGET_CONFIG))

$(SOURCE_DIR)/u-boot.bin: $(SOURCE_DIR)/include/config.h
	@$(call echo_msg,Building RAM loadable U-Boot image...)
	@$(call build_uboot,all)

$(OUTPUT_DIR)/fwupgrade.cfg: $(SOURCE_DIR)/u-boot.bin
	@$(call gen_zero_file,fwupgrade.cfg,196608)
	@$(call combine_files,$(SOURCE_DIR)/u-boot.bin,$(OUTPUT_DIR)/fwupgrade.cfg)

om5p-ac-v2-unlocker: UBOOT_TARGET_CONFIG=board955x_config
om5p-ac-v2-unlocker: UBOOT_TARGET_PARAMS=$(OM5PACV2_RAM_UBOOT_PARAMS)
om5p-ac-v2-unlocker: $(OUTPUT_DIR)/fwupgrade.cfg $(OUTPUT_DIR)/fwupgrade.cfg.sig

clean:
	@$(call echo_msg,Cleaning...)
	@cd $(SOURCE_DIR) && $(SUB_MAKE_CMD) distclean >/dev/null 2>&1
	@rm -f $(OUTPUT_DIR)/fwupgrade.cfg* >/dev/null 2>&1
