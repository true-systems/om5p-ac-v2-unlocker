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
