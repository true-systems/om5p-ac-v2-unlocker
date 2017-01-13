# Define _absolute_ path to your toolchain directory, for example:
export TOOLCHAIN_DIR:=/opt/devel/lede-sdk-ar71xx-generic_gcc-5.4.0_musl-1.1.15.Linux-x86_64/staging_dir/toolchain-mips_24kc_gcc-5.4.0_musl-1.1.15

# Add toolchain to the path
export PATH:=$(TOOLCHAIN_DIR)/bin:$(PATH)

# Add cross compiler prefix
export CROSS_COMPILE=mips-openwrt-linux-
