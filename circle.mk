TOOLCHAIN_NAME ?= toolchain-mips_24kc_gcc-5.4.0_musl-1.1.15
TOOLCHAIN_PREFIX ?= mips-openwrt-linux-
TOOLCHAIN_VERSION ?= lede-sdk-ar71xx-generic_gcc-5.4.0_musl-1.1.15.Linux-x86_64
TOOLCHAIN_TARBALL ?= $(TOOLCHAIN_VERSION).tar.xz
TOOLCHAIN_DL_URL ?= http://ynezz.ibawizard.net/lede/ar71xx/snapshots/$(TOOLCHAIN_TARBALL)
TOOLCHAIN_LOCATION ?= $(TOPDIR)/toolchain/$(TOOLCHAIN_VERSION)/staging_dir/$(TOOLCHAIN_NAME)

.PHONY: ci-prepare ci-test

ci-prepare:
	[ -d toolchain ] || mkdir toolchain
	wget -c $(TOOLCHAIN_DL_URL) -O toolchain/$(TOOLCHAIN_TARBALL)
	tar xJf toolchain/$(TOOLCHAIN_TARBALL) -C toolchain
	echo "export PATH:=$(TOOLCHAIN_LOCATION)/bin:$(PATH)" > local-settings.mk
	echo "export CROSS_COMPILE:=$(TOOLCHAIN_PREFIX)" >> local-settings.mk
	cat local-settings.mk

ci-test:
	make -j`nproc` V=99
	ls -alh output/fwupgrade.cfg output/fwupgrade.cfg.sig
