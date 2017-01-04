include $(TOPDIR)/config.mk

LIB	= lib$(BOARD).a

OBJS	=			\
	$(BOARD).o		\
	flash.o			\
	../common/init-955x.o	\
	../common/serial.o	\
	../common/ath_pci.o	\
	../common/qca-eth-955x.o

#
# The files included in EXTRA_OBJS use some functions that are not
# available in the `bootstrap' portion of tuboot.bin resulting in
# compile/link errors. Hence have them separate (i.e. don't include
# into lib$(BOARD).a and include them only in `u-boot' portion of
# tuboot.bin.
#

ifeq ($(strip $(COMPRESSED_UBOOT)),0) #{
OBJS	+= $(BOARD_EXTRA_OBJS_tmp)
else
EXTRA_OBJS = $(BOARD_EXTRA_OBJS_tmp)
endif #}

ifeq ($(ATH_DUAL_FLASH),1)
OBJS	+= ../common/ath_nand.o ../../../drivers/nand/nand_ids.o
endif

ifndef BOOT_FROM_NAND
OBJS	+= ../common/ath_flash.o
endif

ifeq ($(ATH_RGMII_CAL),1)
OBJS	+= ../common/rgmii-cal-955x.o	
OBJS	+= ../common/athrs17_lpbk.o
endif
ifeq ($(ETH_CONFIG), _vir)
OBJS	+= ../common/athrs_vir_phy.o
endif
ifeq ($(ETH_CONFIG2), _ar8033)
OBJS	+= ../common/athr_ar8033_phy.o
endif

SOBJS	= ../common/955x.o ../common/tap-955x.o

$(LIB):	.depend $(OBJS) $(SOBJS) $(EXTRA_OBJS)
	$(AR) crv $@ $(OBJS) $(SOBJS)

#########################################################################

.depend:	Makefile $(SOBJS:.o=.S) $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(SOBJS:.o=.S) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
