# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
#
# Parts proudly stolen from OpenWRT and modified

SHELL=bash
TOPDIR=${CURDIR}
QUIET=0
export IS_TTY=$(shell tty -s && echo 1 || echo 0)

ifndef OPENWRT_VERBOSE
  OPENWRT_VERBOSE:=
endif
ifeq ("$(origin V)", "command line")
  OPENWRT_VERBOSE:=$(V)
endif

ifeq ($(OPENWRT_VERBOSE),1)
  OPENWRT_VERBOSE:=w
endif
ifeq ($(OPENWRT_VERBOSE),99)
  OPENWRT_VERBOSE:=s
endif

ifeq ($(NO_TRACE_MAKE),)
NO_TRACE_MAKE := $(MAKE) V=s$(OPENWRT_VERBOSE)
export NO_TRACE_MAKE
endif

ifeq ($(IS_TTY),1)
  ifneq ($(strip $(NO_COLOR)),1)
    _G:=\\033[92m
    _R:=\\033[31m
    _N:=\\033[m
  endif
endif

ifeq ($(findstring s,$(OPENWRT_VERBOSE)),)
  define MESSAGE
	printf "$(_G)%s$(_N)\n" "$(1)"
  endef

  define ERROR_MESSAGE
	printf "$(_R)%s$(_N)\n" "$(1)"
  endef

  ifeq ($(QUIET),1)
    ifneq ($(CURDIR),$(TOPDIR))
      _DIR:=$(patsubst $(TOPDIR)/%,%,${CURDIR})
    else
      _DIR:=
    endif
    _NULL:=$(if $(MAKECMDGOALS),$(shell \
		$(call MESSAGE, make[$(MAKELEVEL)]$(if $(_DIR), -C $(_DIR)) $(MAKECMDGOALS)); \
    ))
    SUBMAKE=$(MAKE)
  else
    SILENT:=>/dev/null $(if $(findstring w,$(OPENWRT_VERBOSE)),,2>&1)
    export QUIET:=1
    SUBMAKE=cmd() { $(SILENT) $(MAKE) -s $$* < /dev/null || { echo -e "make $$*\nbuild failed.\nPlease re-run make with -j1 V=s to see what's going on"; false; } } 8>&1 9>&2; cmd
  endif

  .SILENT: $(MAKECMDGOALS)
else
  SUBMAKE=$(MAKE) -w
  define MESSAGE
    printf "%s\n" "$(1)"
  endef
  ERROR_MESSAGE=$(MESSAGE)
endif

#
# Stuff bellow is Copyright (C) 2017 Piotr Dymacz <piotr@dymacz.pl>
#

# $(1): filename
# $(2): size
define gen_zero_file
  @$(call MESSAGE,Generating empty file '$(1)' (size '$(2)')...)
  if [ -e $(OUTPUT_DIR)/$(1) ]; then \
    rm -f $(OUTPUT_DIR)/$(1); \
  fi; \
  $(SILENT) dd if=/dev/zero of=$(OUTPUT_DIR)/$(1) bs=$(2) count=1
endef

# $(1): target
define build_uboot
  cd $(SOURCE_DIR) && \
     $(SUBMAKE) \
     	--no-print-directory V=1 \
	ARCH=mips STAGING_DIR=$(STAGING_DIR) \
	COMPRESSED_UBOOT=0 CROSS_COMPILE=$(CROSS_COMPILE) \
	$(strip $(1)) $(UBOOT_TARGET_PARAMS)
endef

# $(1): input file
# $(2): output file
define combine_files
  @$(call MESSAGE,Combining '$(notdir $1)' and '$(notdir $2)' files...)
  $(SILENT) dd if=$(1) of=$(2) bs=$(shell wc -c < $(1)) \
     count=1 conv=notrunc
endef

# $(1): input file
# $(2): offset
# $(3): value
define swap_byte
  printf "$(shell printf '\\x%02X' $(3))" |\
  $(SILENT) dd of=$(1) bs=1 count=1 seek=$(2) conv=notrunc
endef

ROUTER_IP ?= 192.168.1.100
ROUTER_MODEL ?= unknown
MTD_BACKUP_DIRNAME ?= mtd_backup-$(ROUTER_MODEL)-$(shell date +%Y%m%d%H%M)
MTD_BACKUP_PATH ?= $(TOPDIR)/$(MTD_BACKUP_DIRNAME)
FIND_ART_MTD ?= grep '"ART"' /proc/mtd | cut -c1-4
MTD_PART_LIST ?= $(MTD_BACKUP_PATH)/mtd_partitions.txt

.PHONY: mtd_backup setup_ssh_publickey_auth dump_rsa_pub_key clear_out_rsa_key
mtd_backup:
	$(SILENT) mkdir -p $(MTD_BACKUP_PATH)
	$(SILENT) echo -n > $(MTD_BACKUP_PATH)/md5sums
	$(SILENT) ssh root@$(ROUTER_IP) cat /proc/mtd > $(MTD_PART_LIST)
	set -e; \
	for part in $$(sed 1d $(MTD_PART_LIST) | cut -c1-4); do \
		echo "Creating /dev/$$part backup to $(MTD_BACKUP_PATH)/$$part"; \
		ssh root@$(ROUTER_IP) dd if=/dev/$$part > $(MTD_BACKUP_PATH)/$$part; \
		ssh root@$(ROUTER_IP) "md5sum /dev/$$part | tr '/' ' '| cut -d ' ' -f1,5" >> $(MTD_BACKUP_PATH)/md5sums; \
	done
	cd $(MTD_BACKUP_PATH) && md5sum -c md5sums

setup_ssh_publickey_auth:
	ssh root@$(ROUTER_IP) "tee -a /etc/dropbear/authorized_keys" < $(HOME)/.ssh/id_rsa.pub
	ssh root@$(ROUTER_IP) cat /etc/banner

# 0x8000=offset, 0x20=header, 0x400=keysize
dump_rsa_pub_key:
ifeq ($(ART_PARTITION),)
	$(call ERROR_MESSAGE,Please specify ART_PARTITION parameter)
endif
	[ -f "$(ART_PARTITION)" ] && \
		dd if="$(ART_PARTITION)" bs=$$((0x8000+0x20)) count=1 skip=1 | \
		dd of="$(ART_PARTITION)-rsa.pub.only.key" bs=$$((0x400)) count=1
		dd if="$(ART_PARTITION)" bs=$$((0x8000)) count=1 skip=1 | \
		dd of="$(ART_PARTITION)-rsa.pub.complete.key" bs=$$((0x420)) count=1

# we actually clear out only the header, but it should be enough to cripple the check
clear_out_rsa_key:
	ssh root@$(ROUTER_IP) ls -al /dev/$(FIND_ART_MTD)
	ssh root@$(ROUTER_IP) " \
		dd if=/dev/zero bs=$$((0x20)) count=1 | \
		dd of=/dev/$$($(FIND_ART_MTD))) bs=$$((0x20)) \
			seek=$$((0x8000/0x20)) count=1 conv=notrunc && \
		sync \
		"

unlock:
	make clear_out_rsa_key
	ssh root@$(ROUTER_IP) rm /usr/bin/ng_login

restore_art:
ifeq ($(ART_BACKUP),)
	$(call ERROR_MESSAGE,Please specify ART_BACKUP parameter)
endif
	[ -f "$(ART_BACKUP)" ] && \
	ssh root@$(ROUTER_IP) mtd write - ART < $(ART_BACKUP)
